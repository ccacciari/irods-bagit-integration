# Monitoring rules
#########################################

acPostProcForModifyDataObjMeta()
{
   write_variables("acPostProcForModifyDataObjMeta");
}

acPostProcForModifyAVUMetadata(*Option,*ItemType,*ItemName,*AName,*AValue,*AUnit)
{
   write_var_shortlist("acPostProcForModifyAVUMetadata");
   writeLine("serverLog","acPostProcForModifyAVUMetadata:: "
             ++ "option=" ++ *Option ++ ", "
             ++ "itemType=" ++ *ItemType ++ ", "
             ++ "itemName=" ++ *ItemName ++ ", "
             ++ "AName=" ++ *AName ++ ", "
             ++ "AValue=" ++ *AValue ++ ", "
             ++ "AUnit=" ++ *AUnit
            )
}

write_variables(*event) {
    writeLine("serverLog", *event ++ ":: "
                                  ++ "chksum=$chksum, "
                                  ++ "collId=$collId, "
                                  ++ "connectCnt=$connectCnt, "
                                  ++ "connectOption=$connectOption, "
                                  ++ "connectSock=$connectSock, "
                                  ++ "connectStatus=$connectStatus, "
                                  ++ "dataAccess=$dataAccess, "
                                  ++ "dataAccessInx=$dataAccessInx, "
                                  ++ "dataComments=$dataComments, "
                                  ++ "dataId=$dataId, "
                                  ++ "dataOwner=$dataOwner, "
                                  ++ "dataOwnerZone=$dataOwnerZone, "
                                  ++ "dataSize=$dataSize, "
                                  ++ "dataType=$dataType, "
                                  ++ "destRescName=$destRescName, "
                                  ++ "filePath=$filePath, "
                                  ++ "objPath=$objPath, "
                                  ++ "replNum=$replNum, "
                                  ++ "replStatus=$replStatus, "
                                  ++ "rescName=$rescName, "
                                  ++ "rodsZoneClient=$rodsZoneClient, "
                                  ++ "rodsZoneProxy=$rodsZoneProxy, "
                                  ++ "statusString=$statusString, "
                                  ++ "userNameClient=$userNameClient, "
                                  ++ "userNameProxy=$userNameProxy, "
                                  ++ "version=$version, "
                                  ++ "writeFlag=$writeFlag, "
                                  ++ "clientAddr=$clientAddr, "
                                  ++ "status=$status"
    );
}

write_var_shortlist(*event) {
    writeLine("serverLog", *event ++ ":: "
                                  ++ "connectCnt=$connectCnt, "
                                  ++ "connectOption=$connectOption, "
                                  ++ "connectSock=$connectSock, "
                                  ++ "connectStatus=$connectStatus, "
                                  ++ "rodsZoneClient=$rodsZoneClient, "
                                  ++ "rodsZoneProxy=$rodsZoneProxy, "
                                  ++ "userNameClient=$userNameClient, "
                                  ++ "userNameProxy=$userNameProxy, "
                                  ++ "clientAddr=$clientAddr, "
                                  ++ "status=$status"
    );
}

# Bagit rules
#########################################

SURFcontains(*list,*elem) {
  *ret = false;
  foreach(*e in *list) {
    if(*e == *elem) {
      *ret = true;
    }
  }
*ret;
}


SURFgroupDataByResource(*default_resc, *path, *default_resc_list, *data_list) {

  msiMakeGenQuery("DATA_NAME, RESC_NAME", "COLL_NAME = '*path'", *genQIn);
  msiExecGenQuery(*genQIn, *genQOut);
  foreach(*genQOut){
    msiGetValByKey(*genQOut, "RESC_NAME", *resc);
    msiGetValByKey(*genQOut, "DATA_NAME", *data_name);
    *data = list();
    if (*resc == *default_resc) {
      *default_resc_list = cons(*data_name, *default_resc_list);
    }
    else {
      *data = list(*data_name,*resc);
      *data_list = cons(*data, *data_list);
    }
  }
}


SURFphymoveDataPerCollection(*default_resc, *path, *default_resc_list, *data_list) {

    for(*i=0; *i<size(*data_list); *i=*i+1) {
      *data_name = hd(elem(*data_list, *i));
      *data_res = elem(elem(*data_list, *i), 1);
      *data_path = *path ++ "/" ++ *data_name
      if (SURFcontains(*default_resc_list, *data_name)) {
        msiDataObjTrim("*data_path", "*data_res", "null", "1", "IRODS_ADMIN", *status);
      }
      else {
        msiDataObjPhymv("*data_path", "*default_resc", "*data_res", "null", "IRODS_ADMIN", *status)
        *default_resc_list = cons(*data_name, *default_resc_list)
      }
    }
}
