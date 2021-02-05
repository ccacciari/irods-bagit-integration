# Bagit rules
#########################################

SURFcontains(*list,*elem) {
  # check if *elem is included in the list *list
  *ret = false;
  foreach(*e in *list) {
    if(*e == *elem) {
      *ret = true;
    }
  }
*ret;
}


SURFgetOwners(*path) {
  # get all the users who are owners of the collection/object
  *users = list()
  msiGetObjType(*path, *type);
  if (*type == "-c") {
    # the query for the collection
    msiMakeGenQuery("USER_NAME, USER_ZONE", "COLL_NAME = '*path' AND COLL_ACCESS_NAME = 'own'", *genQIn1);
  }
  else if (*type == "-d") {
    # the query for the object
    msiSplitPath(*path, *coll, *object);
    msiMakeGenQuery("USER_NAME, USER_ZONE", "COLL_NAME = '*coll' AND DATA_NAME = '*object' AND DATA_ACCESS_NAME = 'own'", *genQIn1);
  }
  msiExecGenQuery(*genQIn1, *genQOut1);
  foreach(*genQOut1){
    # loop over all the users and create a list
    msiGetValByKey(*genQOut1, "USER_NAME", *user_name);
    msiGetValByKey(*genQOut1, "USER_ZONE", *user_zone);
    *user = *user_name ++ "#" ++ *user_zone;
    *users = cons(*user, *users);
  }

  *users
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


SURFbagit(*coll_path, *source_res, *dest_res, *owner, *admin_user, *users, *cmd_name, *vault_path, *op) {

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
  # get the list of requested operations
  *operations = split("*op","::");
  *transfer_option = hd(*operations);
  *compress = "tgz";
  *metadata = "";
  if (size(tl(*operations)) > 0) {
    *operations = tl(*operations);
    *compress = hd(*operations);
  }
  if (size(tl(*operations)) > 0) {
    *operations = tl(*operations);
    *metadata = hd(*operations);
  }
  # set the flag about removing the original collection
  if (*transfer_option == 'move') { *flag = 'true' }
  else { 
    *flag = 'false';
  }

  # give the right permissions to the admin
  msiSetACL("default", "admin:own", "*admin_user", *parent_coll);
  msiSetACL("recursive", "admin:own", "*admin_user", *coll_path);

  # in case of copy, check if the target package is already there
  *skip_bagit = 'false';
  if (*flag == 'false') {
    *target_obj = "*coll_path" ++ ".*compress";
    if (errorcode(msiObjStat(*target_obj, *stat)) >= 0) {
      *skip_bagit = 'true';
      *Out = "bagit package [*target_obj] already exists, nothing to do."
    }
  }
  if (*skip_bagit == 'false') {
    *msi_err = errorcode(msiExecCmd(*cmd_name, "*abs_path *coll_path *source_res *dest_res *flag *compress *metadata", 
                                    "*res_loc", "null", "null", *Result));
    if (*msi_err >= 0) {
      msiGetStdoutInExecCmdOut(*Result, *Out);
    }
    else {
      msiGetStderrInExecCmdOut(*Result, *Out);
    }
  }

  # restore the original permissions
  foreach(*user in *users) {
    msiSetACL("default", "admin:own", "*user", "*coll_path" ++ ".*compress");
  }
  if (size(*users) == 0) {
    msiSetACL("default", "admin:own", "*owner", "*coll_path" ++ ".*compress");
  }
  if (! SURFcontains(*users,*admin_user)) {
    if (*flag == 'false') {
      msiSetACL("recursive", "admin:null", "*admin_user", *coll_path);
    }
    msiSetACL("default", "admin:null", "*admin_user", "*coll_path" ++ ".*compress");
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
  msiMakeGenQuery("COLL_NAME, COLL_OWNER_NAME, COLL_OWNER_ZONE, META_COLL_ATTR_NAME, META_COLL_ATTR_VALUE, META_COLL_ATTR_UNITS", 
                  "META_COLL_ATTR_NAME = 'SURFbagit' AND COLL_NAME not like '/%/trash/%'", *genQIn);
  msiExecGenQuery(*genQIn, *genQOut);
  foreach(*genQOut){
    msiGetValByKey(*genQOut, "COLL_NAME", *path);
    msiGetValByKey(*genQOut, "META_COLL_ATTR_NAME", *name);
    msiGetValByKey(*genQOut, "META_COLL_ATTR_VALUE", *dest_res);
    msiGetValByKey(*genQOut, "META_COLL_ATTR_UNITS", *operation);
    msiGetValByKey(*genQOut, "COLL_OWNER_NAME", *owner_name);
    msiGetValByKey(*genQOut, "COLL_OWNER_ZONE", *owner_zone);

    *owner = *owner_name ++ "#" ++ *owner_zone
    *users = SURFgetOwners(*path)    

    *resp = SURFbagit(*path, *default_resc, *dest_res, *owner, *admin_user, *users, *cmd, *vault_path, *operation);
    writeLine("serverLog", "[SURFbagitBatch] *resp");
    *response = *response ++ "*resp" ++ "; ";
  }

  *response;
}


SURFunbagit(*bag_path, *source_res, *dest_res, *owner, *admin_user, *users, *cmd_name, *op) {

  writeLine("serverLog", "[SURFunbagit] collection: *bag_path, source resource: *source_res, "
           ++ "destination resource: *dest_res, admin user: *admin_user, command: *cmd_name, "
           ++ "operation: *op");

  # get vault path of destination resource
  msiMakeGenQuery("RESC_VAULT_PATH", "RESC_NAME = '*dest_res'", *genQIn0);
  msiExecGenQuery(*genQIn0, *genQOut0);
  foreach(*genQOut0){
    msiGetValByKey(*genQOut0, "RESC_VAULT_PATH", *vault_path_dest);
  }

  # get the file system absolute path of the collection
  *tokens = split("*bag_path","/");
  *rel_path = "";
  foreach(*E in tl(*tokens)) {
    *rel_path = *rel_path ++ "/" ++ *E;
  }
  *abs_path = *vault_path_dest ++ *rel_path;

  # get the hostname of the destination resource
  msiMakeGenQuery("RESC_LOC", "RESC_NAME = '*dest_res'", *genQIn1);
  msiExecGenQuery(*genQIn1, *genQOut1);
  foreach(*genQOut1){
    msiGetValByKey(*genQOut1, "RESC_LOC", *res_loc);
  }

  # get the parent collection
  *parent_coll = trimr("*bag_path", "/");

  # set the flag about removing the original collection
  if (*op == 'move') { *flag = 'true' }
  else { 
    *flag = 'false';
  }

  # give the right permissions to the admin
  msiSetACL("default", "admin:own", "*admin_user", *parent_coll);
  msiSetACL("default", "admin:own", "*admin_user", *bag_path);

  *msi_err = errorcode(msiExecCmd(*cmd_name, "*abs_path *bag_path *source_res *dest_res *flag", "*res_loc", "null", "null", *Result));
  if (*msi_err >= 0) {
    msiGetStdoutInExecCmdOut(*Result, *Out);
  }
  else {
    msiGetStderrInExecCmdOut(*Result, *Out);
  }

  if(*bag_path like "*.tar") {
	*coll_path = trimr(*bag_path,".tar")
  }
  else if(*bag_path like "*.tgz") {
	*coll_path = trimr(*bag_path,".tgz")
  }
  else if(*bag_path like "*.zip") {
	*coll_path = trimr(*bag_path,".zip")
  }
  else {
	*coll_path = *bag_path
  }

  # restore the original permissions
  foreach(*user in *users) {
    msiSetACL("recursive", "admin:own", "*user", "*coll_path");
  }
  if (size(*users) == 0) {
    msiSetACL("recursive", "admin:own", "*owner", "*coll_path");
  }
  if (! SURFcontains(*users,*admin_user)) {
    if (*flag == 'false') {
      msiSetACL("default", "admin:null", "*admin_user", *bag_path);
    }
    msiSetACL("recursive", "admin:null", "*admin_user", "*coll_path");
    msiSetACL("default", "admin:null", "*admin_user", *parent_coll);
  }

  *Out
}


SURFunbagitBatch(*default_resc, *admin_user, *cmd) {

  writeLine("serverLog", "[SURFunbagitBatch] default resource: *default_resc, admin: *admin_user, command: *cmd");

  *response = "";
  msiMakeGenQuery("COLL_NAME, DATA_NAME, DATA_OWNER_NAME, DATA_OWNER_ZONE, META_DATA_ATTR_NAME, META_DATA_ATTR_VALUE, META_DATA_ATTR_UNITS", 
                  "META_DATA_ATTR_NAME = 'SURFunbagit'", *genQIn);
  msiExecGenQuery(*genQIn, *genQOut);
  foreach(*genQOut){
    msiGetValByKey(*genQOut, "COLL_NAME", *coll_path);
    msiGetValByKey(*genQOut, "DATA_NAME", *bag_file);
    msiGetValByKey(*genQOut, "META_DATA_ATTR_NAME", *name);
    msiGetValByKey(*genQOut, "META_DATA_ATTR_VALUE", *dest_res);
    msiGetValByKey(*genQOut, "META_DATA_ATTR_UNITS", *operation);
    msiGetValByKey(*genQOut, "DATA_OWNER_NAME", *owner_name);
    msiGetValByKey(*genQOut, "DATA_OWNER_ZONE", *owner_zone);

    *owner = *owner_name ++ "#" ++ *owner_zone
    *path = *coll_path ++ "/" ++ *bag_file
    *users = SURFgetOwners(*path)

    *resp = SURFunbagit(*path, *default_resc, *dest_res, *owner, *admin_user, *users, *cmd, *operation);
    writeLine("serverLog", "[SURFunbagitBatch] *resp");
    *response = *response ++ "*resp" ++ "; ";
  }

  *response;
}

