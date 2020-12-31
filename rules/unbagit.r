unbagitRule {

  *DEFAULT_RESC = 'surfArchive'
  *ADMIN_USER = 'rods#surfZone'
  *CMD = 'unbagit'

  *response = SURFunbagitBatch(*DEFAULT_RESC, *ADMIN_USER, *CMD);
  writeLine("stdout", "*response");

}
INPUT null
OUTPUT ruleExecOut
