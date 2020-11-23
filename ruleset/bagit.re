# Bagit rules
#########################################

SURFcontains(*list,*elem) {
  *ret = false;
  foreach(*e in *list) {
    if(*e == *elem) {
      *ret = true;
    }
  }
*ret;
}


SURFgroupDataByResource(*default_resc, *path, *default_resc_list, *data_list) {

  writeLine("serverLog", "[SURFgroupDataByResource] default resource: *default_resc, collection: *path");
  # get all the objects contained in the collection
  msiMakeGenQuery("DATA_NAME, RESC_NAME", "COLL_NAME = '*path'", *genQIn);
  msiExecGenQuery(*genQIn, *genQOut);
  foreach(*genQOut){
    msiGetValByKey(*genQOut, "RESC_NAME", *resc);
    msiGetValByKey(*genQOut, "DATA_NAME", *data_name);
    *data = list();
    # group all the objects in the default resource
    if (*resc == *default_resc) {
      *default_resc_list = cons(*data_name, *default_resc_list);
    }
    # group all the objects in the other resources
    else {
      *data = list(*data_name,*resc);
      *data_list = cons(*data, *data_list);
    }
  }
}


SURFphymoveDataPerCollection(*default_resc, *path, *default_resc_list, *data_list) {

    writeLine("serverLog", "[SURFphymoveDataPerCollection] default resource: *default_resc, collection: *path");
    # loop over all the objects in other resources
    for(*i=0; *i<size(*data_list); *i=*i+1) {
      *data_name = hd(elem(*data_list, *i));
      *data_res = elem(elem(*data_list, *i), 1);
      *data_path = *path ++ "/" ++ *data_name
      # if the object is already stored in the default resource, trim the other copy
      if (SURFcontains(*default_resc_list, *data_name)) {
        msiDataObjTrim("*data_path", "*data_res", "null", "1", "IRODS_ADMIN", *status);
      }
      # if not, move it to the default resource
      else {
        msiDataObjPhymv("*data_path", "*default_resc", "*data_res", "null", "IRODS_ADMIN", *status)
        *default_resc_list = cons(*data_name, *default_resc_list)
      }
    }
}


SURFbagitAlignDataResources(*coll_path, *dest_res) {

  writeLine("serverLog", "[SURFbagitAlignDataResources] collection: *coll_path");
  *data_list = list()
  *default_resc_list = list()
  SURFgroupDataByResource(*dest_res, *coll_path, *default_resc_list, *data_list);
  SURFphymoveDataPerCollection(*dest_res, *coll_path, *default_resc_list, *data_list);
  *search_key = "*coll_path" ++ "/%"
  msiMakeGenQuery("COLL_NAME", "COLL_NAME like '*search_key'", *genQIn);
  msiExecGenQuery(*genQIn, *genQOut);
  foreach(*genQOut){
    msiGetValByKey(*genQOut, "COLL_NAME", *path);
    SURFbagitAlignDataResources(*path, *dest_res);
  }
}


SURFbagit(*coll_path, *source_res, *dest_res, *owner, *admin_user, *cmd_name, *vault_path, *op) {

  writeLine("serverLog", "[SURFbagit] collection: *coll_path, source resource: *source_res, "
           ++ "destination resource: *dest_res, admin user: *admin_user, command: *cmd_name, "
           ++ "vault: *vault_path, operation: *op");
  # move all the data to a single resource, recursively through all the sub-collections
  SURFbagitAlignDataResources(*coll_path, *source_res);

  # get the file system absolute path of the collection
  *tokens = split("*coll_path","/");
  *rel_path = "";
  foreach(*E in tl(*tokens)) {
    *rel_path = *rel_path ++ "/" ++ *E;
  }
  *abs_path = *vault_path ++ *rel_path;

  # get the hostname of the destination resource
  msiMakeGenQuery("RESC_LOC", "RESC_NAME = '*dest_res'", *genQIn1);
  msiExecGenQuery(*genQIn1, *genQOut1);
  foreach(*genQOut1){
    msiGetValByKey(*genQOut1, "RESC_LOC", *res_loc);
  }

  # get the parent collection
  *parent_coll = trimr("*coll_path", "/");
  # set the flag about removing the original collection
  if (*op == 'move') { *flag = 'true' }
  else { 
    *flag = 'false';
  }

  # give the right permissions to the admin
  if (*owner != *admin_user) {
    msiSetACL("default", "admin:own", "*admin_user", *parent_coll);
    msiSetACL("recursive", "admin:own", "*admin_user", *coll_path);
  }

  # in case of copy, check if the target package is already there
  *skip_bagit = 'false';
  if (*flag == 'false') {
    *target_obj = "*coll_path" ++ ".tgz";
    if (errorcode(msiObjStat(*target_obj, *stat)) >= 0) {
      *skip_bagit = 'true';
      *Out = "bagit package [*target_obj] already exists, nothing to do."
    }
  }
  if (*skip_bagit == 'false') {
    *msi_err = errorcode(msiExecCmd(*cmd_name, "*abs_path *coll_path *source_res *dest_res *flag", "*res_loc", "null", "null", *Result));
    if (*msi_err >= 0) {
      msiGetStdoutInExecCmdOut(*Result, *Out);
    }
    else {
      msiGetStderrInExecCmdOut(*Result, *Out);
    }
  }

  # restore the original permissions
  if (*owner != *admin_user) {
    if (*flag == 'false') {
      msiSetACL("recursive", "admin:null", "*admin_user", *coll_path);
    }
    msiSetACL("default", "admin:own", "*owner", "*coll_path" ++ ".tgz");
    msiSetACL("default", "admin:null", "*admin_user", "*coll_path" ++ ".tgz");
    msiSetACL("default", "admin:null", "*admin_user", *parent_coll);
  }

  *Out
}


SURFbagitBatch(*default_resc, *admin_user, *cmd) {

  writeLine("serverLog", "[SURFbagitBatch] default resource: *default_resc, admin: *admin_user, command: *cmd");
  msiMakeGenQuery("RESC_VAULT_PATH", "RESC_NAME = '*default_resc'", *genQIn0);
  msiExecGenQuery(*genQIn0, *genQOut0);
  foreach(*genQOut0){
    msiGetValByKey(*genQOut0, "RESC_VAULT_PATH", *vault_path);
  }

  *response = "";
  msiMakeGenQuery("COLL_NAME, COLL_OWNER_NAME, META_COLL_ATTR_NAME, META_COLL_ATTR_VALUE, META_COLL_ATTR_UNITS", "META_COLL_ATTR_NAME = 'SURFbagit'", *genQIn);
  msiExecGenQuery(*genQIn, *genQOut);
  foreach(*genQOut){
    msiGetValByKey(*genQOut, "COLL_NAME", *path);
    msiGetValByKey(*genQOut, "META_COLL_ATTR_NAME", *name);
    msiGetValByKey(*genQOut, "META_COLL_ATTR_VALUE", *dest_res);
    msiGetValByKey(*genQOut, "META_COLL_ATTR_UNITS", *operation);
    msiGetValByKey(*genQOut, "COLL_OWNER_NAME", *owner);

    *resp = SURFbagit(*path, *default_resc, *dest_res, *owner, *admin_user, *cmd, *vault_path, *operation);
    writeLine("serverLog", "[SURFbagitBatch] *resp");
    *response = *response ++ "*resp" ++ "; ";
  }

  *response;
}
