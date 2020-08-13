#!/bin/sh

FS_PATH=$1
IRODS_PATH=$2
IRODS_SOURCE_RES=$3
IRODS_TARGET_RES=$4
REMOVE_SOURCE=$5

BDBAG=$(/usr/bin/which bdbag)

# create the package
${BDBAG} ${FS_PATH} --archive tgz 
# if the package creation fails, exit
if [ "$?" -ne "0" ]; then
  echo "Failure: ${BDBAG} ${FS_PATH} --archive tgz"
  exit 1
fi

# register the package
/bin/ireg -f -K "${FS_PATH}.tgz" "${IRODS_PATH}.tgz" 
# if the registration fails, 
if [ "$?" -ne "0" ]; then
  echo "Failure: /bin/ireg -f -K ${FS_PATH}.tgz ${IRODS_PATH}.tgz"
  # remove the package
  /bin/rm -r "${FS_PATH}.tgz"
  # revert back the collection
  ${BDBAG} --revert ${FS_PATH}
  exit 1
# if the registration succeeds, move the package to the target resource
else
  /bin/iphymv -M -S ${IRODS_SOURCE_RES} -R ${IRODS_TARGET_RES} "${IRODS_PATH}.tgz"
fi

# revert back the collection
${BDBAG} --revert ${FS_PATH}
if [ "$?" -ne "0" ]; then
  echo "Failure: ${BDBAG} --revert ${FS_PATH}"
  exit 1
fi

# remove both the iRODS collection and the FS folder
if [ "$REMOVE_SOURCE" = true ]; then
  /bin/irm -f -r ${IRODS_PATH}
  /bin/rmdir ${FS_PATH}
fi

exit 0
