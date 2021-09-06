unbagitRule {

  *CMD = 'unbagit'

  *response = SURFunbagitBatch(*DEFAULT_RESC, *ADMIN_USER, *CMD);
  writeLine("stdout", "*response");

}
INPUT *DEFAULT_RESC="demoResc", *ADMIN_USER="rods#surfZone"
OUTPUT ruleExecOut
