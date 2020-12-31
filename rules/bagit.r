bagitRule {

  *DEFAULT_RESC = 'demoResc'
  *ADMIN_USER = 'rods#surfZone'
  *CMD = 'bagit'

  *response = SURFbagitBatch(*DEFAULT_RESC, *ADMIN_USER, *CMD);
  writeLine("stdout", "*response");

}
INPUT null
OUTPUT ruleExecOut
