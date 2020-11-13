bagitRule {

  *DEFAULT_RESC = 'innerResc'

  msiMakeGenQuery("RESC_VAULT_PATH", "RESC_NAME = '*DEFAULT_RESC'", *genQIn0);
  msiExecGenQuery(*genQIn0, *genQOut0);
  foreach(*genQOut0){
    msiGetValByKey(*genQOut0, "RESC_VAULT_PATH", *vault_path);
  }

  msiMakeGenQuery("COLL_NAME, COLL_OWNER_NAME, META_COLL_ATTR_NAME, META_COLL_ATTR_VALUE, META_COLL_ATTR_UNITS", "META_COLL_ATTR_NAME = 'SURFbagit'", *genQIn);
  msiExecGenQuery(*genQIn, *genQOut);
  foreach(*genQOut){
    msiGetValByKey(*genQOut, "COLL_NAME", *path);
    msiGetValByKey(*genQOut, "META_COLL_ATTR_NAME", *name);
    msiGetValByKey(*genQOut, "META_COLL_ATTR_VALUE", *dest_res);
    msiGetValByKey(*genQOut, "META_COLL_ATTR_UNITS", *operation);
    msiGetValByKey(*genQOut, "COLL_OWNER_NAME", *owner);

    *data_list = list()
    *default_resc_list = list()
    SURFgroupDataByResource(*DEFAULT_RESC, *path, *default_resc_list, *data_list);
    SURFphymoveDataPerCollection(*DEFAULT_RESC, *path, *default_resc_list, *data_list);
  }

  *tokens = split("*path","/");
  *rel_path = ""
  foreach(*E in tl(*tokens)) {
    *rel_path = *rel_path ++ "/" ++ *E
  }

  msiMakeGenQuery("RESC_LOC", "RESC_NAME = '*dest_res'", *genQIn1);
  msiExecGenQuery(*genQIn1, *genQOut1);
  foreach(*genQOut1){
    msiGetValByKey(*genQOut1, "RESC_LOC", *res_loc);
  }  

  *parent_coll = trimr("*path", "/")
  msiSetACL("default", "admin:own", "irodsmaster", *parent_coll)
  msiSetACL("recursive", "admin:own", "irodsmaster", *path)

  *abs_path = *vault_path ++ *rel_path;
  *Cmd="bagit";
  if (*operation == 'move') { *flag = 'true' }
  else { *flag = 'false' }
  msiExecCmd(*Cmd, "*abs_path *path *DEFAULT_RESC *dest_res *flag", "*res_loc", "null", "null", *Result);
  msiGetStdoutInExecCmdOut(*Result, *Out);
  if (*flag == 'false') {
    msiSetACL("recursive", "admin:null", "irodsmaster", *path)
  }
  msiSetACL("default", "admin:own", "*owner", "*path" ++ ".tgz")
  msiSetACL("default", "admin:null", "irodsmaster", "*path" ++ ".tgz")
  msiSetACL("default", "admin:null", "irodsmaster", *parent_coll)  

  writeLine("stdout", "Result: *Out");

}
INPUT null
OUTPUT ruleExecOut
