# Introduction

This project contains scripts and rules to enable iRODS to use bagit collections and objects.  
The file packaging format "bagit" is a IETF standard: https://datatracker.ietf.org/doc/rfc8493.   
In this project the following library/CLI tool is used to deal with bagit folders and files: https://github.com/fair-research/bdbag  
The main objective of the project is to provide an utility that enables iRODS (irods.org) to pack and unpack bagit compliant packages.  
  
## Deployment
Deploy the ruleset bagit.re in the folder /etc/irods and add it to the server_config file.  
See the example rule bagit.r about how to call the bagit rule.  
The rule SURFbagitBatch searches all the collections with the matadata attribute "SURFbagit" and it expects the destination resource as a value and one of the two keywords "copy" or "move" as units. If the word "copy" is set then the folder is archived, but the original copy is still available, otherwise it is removed.  

```
[claudio@145 ~]$ imeta set -C /surfTestZone2/home/claudio/mylibrary2 SURFbagit archive copy
[claudio@145 ~]$ imeta ls -C /surfTestZone2/home/claudio/mylibrary2
AVUs defined for collection /surfTestZone2/home/claudio/mylibrary2:
attribute: SURFbagit
value: archive
units: copy
```


The two scripts "bagit" and "unbagit" must be placed in /var/lib/irods/msiExecCmd_bin.
Both the rules SURFbagit and SURFbagitBatch must be executed by a user with administrator privileges.

## TODO
The unbagit rule can not be executed in asynchronous mode yet.  
Only for the unbagit process, the admin user is still forced to manually change the permission on the target folder and restore the original permissions on the archived bagit package.  
Performance optimization has not been investigated, in particular about the checksum computation with bdbag.  
