#!/bin/sh

FS_PATH=$1
IRODS_PATH=$2
IRODS_SOURCE_RES=$3
IRODS_TARGET_RES=$4
REMOVE_SOURCE=$5

tokens=($(echo ${IRODS_PATH} | tr "." "\n"))
IRODS_COLL_PATH=${tokens[0]}
tokens=($(echo ${FS_PATH} | tr "." "\n"))
FS_COLL_PATH=${tokens[0]}
if [ "$REMOVE_SOURCE" = true ]; then
  echo "move ${IRODS_PATH} from ${IRODS_SOURCE_RES} to ${IRODS_TARGET_RES}"
  /bin/iphymv -M -S ${IRODS_SOURCE_RES} -R ${IRODS_TARGET_RES} "${IRODS_PATH}"
  echo "bdbag --materialize ${FS_PATH}"
  /home/irodsmaster/.local/bin/bdbag --materialize "${FS_PATH}"
  echo "remove ${IRODS_PATH}"
  /bin/irm -f "${IRODS_PATH}"
else
  echo "replicate ${IRODS_PATH} from ${IRODS_SOURCE_RES} to ${IRODS_TARGET_RES}"
  /bin/irepl -M -S ${IRODS_SOURCE_RES} -R ${IRODS_TARGET_RES} ${IRODS_PATH}
  echo "bdbag --materialize ${FS_PATH}"
  /home/irodsmaster/.local/bin/bdbag --materialize "${FS_PATH}"
  echo "trim ${IRODS_PATH} in ${IRODS_TARGET_RES}"
  /bin/itrim -M -S ${IRODS_TARGET_RES} -N 1 ${IRODS_PATH}
fi
echo "bdbag --revert ${FS_COLL_PATH}"
/home/irodsmaster/.local/bin/bdbag --revert "${FS_COLL_PATH}"
echo "register the collection ${FS_COLL_PATH} as ${IRODS_COLL_PATH}"
/bin/ireg -f -C -K "${FS_COLL_PATH}" "${IRODS_COLL_PATH}"

exit 0
