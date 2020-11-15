bagitRule {

  *DEFAULT_RESC = 'innerResc'
  *ADMIN_USER = 'irodsmaster'
  *CMD = 'bagit'

  *response = SURFbagitBatch(*DEFAULT_RESC, *ADMIN_USER, *CMD);
  writeLine("stdout", "*response");

}
INPUT null
OUTPUT ruleExecOut
