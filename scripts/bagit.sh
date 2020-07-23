#!/bin/sh

FS_PATH=$1
IRODS_PATH=$2
IRODS_SOURCE_RES=$3
IRODS_TARGET_RES=$4
REMOVE_SOURCE=$5

echo "bdbag ${FS_PATH} --archive tgz"
/home/irodsmaster/.local/bin/bdbag ${FS_PATH} --archive tgz
echo "bdbag ${FS_PATH} --revert ${FS_PATH}"
/home/irodsmaster/.local/bin/bdbag --revert ${FS_PATH}
if [ "$REMOVE_SOURCE" = true ]; then
  echo "remove collection ${IRODS_PATH}"
  /bin/irm -f -r ${IRODS_PATH}
  echo "remove collection ${FS_PATH}"
  /bin/rm -r ${FS_PATH}
fi
echo "register bagit compressed package ${FS_PATH}.tgz as ${IRODS_PATH}.tgz"
/bin/ireg -f -K "${FS_PATH}.tgz" "${IRODS_PATH}.tgz"
echo "move ${IRODS_PATH}.tgz from ${IRODS_SOURCE_RES} to ${IRODS_TARGET_RES}"
/bin/iphymv -M -S ${IRODS_SOURCE_RES} -R ${IRODS_TARGET_RES} "${IRODS_PATH}.tgz"

exit 0
