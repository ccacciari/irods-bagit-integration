#!/bin/sh

username=$1
if [ -z "${username}" ]; then
  echo "usage: $0 <username> [defaultResc targetResc], please pass an iRODS username as input"
  exit 0
fi
defaultResc=$2
if [ -z "${defaultResc}" ]; then
  echo "assuming default resource = demoResc"
  defaultResc="demoResc"
fi
targetResc=$3
if [ -z "${targetResc}" ]; then
  echo "assuming target resource = demoResc"
  targetResc="demoResc"
fi

# get the type of the iRODS user running this script
utype=$(iuserinfo | grep type)
utype=${utype##*: }
if [ "${utype}" != "rodsadmin" ]; then 
  echo "you are not a user of type rodsadmin, then I will stop"
  exit 0
else
  # get the username of the iRODS user running this script
  admin=$(iuserinfo | grep name)
  admin=${admin##*: }
  # get the zone of the iRODS user running this script
  zone=$(iuserinfo | grep zone)
  zone=${zone##*: }  
fi

# check if a normal rodsuser is available for the test
response=$(iadmin lu | grep ${username});
if [ -z "${response}" ]; then
  iadmin mkuser "${username}" rodsuser
  usercreated=true
fi  
# alias the user
export clientUserName="${username}"

# create a file
echo "Sator arepo tenet opera rotas" > testobj.txt
# go to the home folder
icd /${zone}/home/${username}
# create a collection in iRODS
imkdir testcollection
# upload a file to the collection
iput testobj.txt testcollection/
# add metadata to the collection to trigger the packaging process
imeta set -C testcollection SURFbagit ${targetResc} copy
# add metadata to the data object
imeta set -d testcollection/testobj.txt title "magic square" latin

# unalias the user
unset clientUserName

# trigger the bagit operation
irule -r irods_rule_engine_plugin-irods_rule_language-instance -F rules/bagit.r "*DEFAULT_RESC='${defaultResc}'" "*ADMIN_USER='${admin}'"

# alias the user
export clientUserName="${username}"

# go to the home folder
icd /${zone}/home/${username}
# remove the collection
irm -rf testcollection

# add metadata to the bagit package to trigger the unpackaging process
imeta set -d testcollection.tgz SURFunbagit ${targetResc} move

# unalias the user
unset clientUserName

# trigger the unbagit operation
irule -r irods_rule_engine_plugin-irods_rule_language-instance -F rules/unbagit.r "*DEFAULT_RESC='${defaultResc}'" "*ADMIN_USER='${admin}'"

# alias the user
export clientUserName="${username}"

# go to the home folder
icd /${zone}/home/${username}
# remove the collection
irm -rf testcollection
# remove from trash
irmtrash
# remove the file
rm testobj.txt

# unalias the user
unset clientUserName

# remove the created user
if [ "${usercreated}" = true ]; then
  iadmin rmuser "${username}"
fi


