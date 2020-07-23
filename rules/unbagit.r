bagitRule {

  *tokens = split("*Coll","/");
  *rel_path = ""
  foreach(*E in tl(*tokens)) {
    *rel_path = *rel_path ++ "/" ++ *E
  }

  msiMakeGenQuery("RESC_VAULT_PATH", "RESC_NAME = '*dest_res'", *genQIn);
  msiExecGenQuery(*genQIn, *genQOut);
  foreach(*genQOut){
    msiGetValByKey(*genQOut, "RESC_VAULT_PATH", *vault_path);
  }
  *abs_path = *vault_path ++ *rel_path;

  msiExecCmd(*Cmd, "*abs_path *Coll *src_res *dest_res true", "null", "null", "null", *Result);
  msiGetStdoutInExecCmdOut(*Result, *Out);
  writeLine("stdout", "Result: *Out");

}
INPUT *Cmd="unbagit.sh", *Coll="/surfTestZone2/home/irodsmaster/indexed_collection/sherlock.tgz", *src_res="archive", *dest_res="innerResc"
OUTPUT ruleExecOut
