#!/bin/sh

FS_PATH=$1
IRODS_PATH=$2
IRODS_SOURCE_RES=$3
IRODS_TARGET_RES=$4
REMOVE_SOURCE=$5

BDBAG=$(/usr/bin/which bdbag)

tokens=($(echo ${IRODS_PATH} | tr "." "\n"))
IRODS_COLL_PATH=${tokens[0]}
tokens=($(echo ${FS_PATH} | tr "." "\n"))
FS_COLL_PATH=${tokens[0]}

# replicate the package
/bin/irepl -M -S ${IRODS_SOURCE_RES} -R ${IRODS_TARGET_RES} ${IRODS_PATH}
 # if the replication fails, exit
if [ "$?" -ne "0" ]; then
  echo "Failure: /bin/irepl -M -S ${IRODS_SOURCE_RES} -R ${IRODS_TARGET_RES} ${IRODS_PATH}"
  exit 1
fi
# materialize the package into a collection
${BDBAG} --materialize "${FS_PATH}"
# revert the collection structure
${BDBAG} --revert "${FS_COLL_PATH}"
# register the collection
/bin/ireg -f -C -K "${FS_COLL_PATH}" "${IRODS_COLL_PATH}"
# if the registration fails,
if [ "$?" -ne "0" ]; then
  echo "Failure: /bin/ireg -f -C -K ${FS_COLL_PATH} ${IRODS_COLL_PATH}"
  # remove the collection
  /bin/rmdir "${FS_COLL_PATH}"
  # remove the replica
  /bin/itrim -M -S ${IRODS_TARGET_RES} -N 1 ${IRODS_PATH}
  exit 1
fi

if [ "$REMOVE_SOURCE" = true ]; then
  /bin/irm -f "${IRODS_PATH}"
else
  /bin/itrim -M -S ${IRODS_TARGET_RES} -N 1 ${IRODS_PATH}
fi

exit 0
