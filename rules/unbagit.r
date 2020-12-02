unbagitRule {

  *DEFAULT_RESC = 'irodsResc2'
  *ADMIN_USER = 'rods'
  *CMD = 'unbagit'

  *response = SURFunbagitBatch(*DEFAULT_RESC, *ADMIN_USER, *CMD);
  writeLine("stdout", "*response");

}
INPUT null
OUTPUT ruleExecOut
