bagitRule {

  *tokens = split("*Coll","/");
  *rel_path = ""
  foreach(*E in tl(*tokens)) {
    *rel_path = *rel_path ++ "/" ++ *E
  }

  msiMakeGenQuery("RESC_VAULT_PATH", "COLL_NAME = '*Coll' AND RESC_NAME = '*src_res'", *genQIn);
  msiExecGenQuery(*genQIn, *genQOut);
  foreach(*genQOut){
    msiGetValByKey(*genQOut, "RESC_VAULT_PATH", *vault_path);
  }
  *abs_path = *vault_path ++ *rel_path;

  msiExecCmd(*Cmd, "*abs_path *Coll *src_res *dest_res true", "null", "null", "null", *Result);
  msiGetStdoutInExecCmdOut(*Result, *Out);
  writeLine("stdout", "Result: *Out");

}
INPUT *Cmd="bagit.sh", *Coll="/surfTestZone2/home/irodsmaster/indexed_collection/sherlock", *src_res="innerResc", *dest_res="archive"
OUTPUT ruleExecOut
