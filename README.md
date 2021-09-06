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

The rule SURFunbagitBatch works in a similar way, but applied to bagit archives:

```
$ imeta set -d mylibrary.zip SURFunbagit demoResc move
```

The two scripts "bagit" and "unbagit" must be placed in /var/lib/irods/msiExecCmd_bin.  
The rules SURFbagit, SURFunbagit, SURFbagitBatch and SURFunbagitBatch must be executed by a user with administrator privileges.  

## Compression
It is possible to choose the archive format among three options: zip, tgz, tar. Adding the desired option as metadata:  

```
$ imeta set -C /surfTestZone2/home/claudio/mylibrary2 SURFbagit archive copy::zip
```

This option is taken into account only by the SURFbagitBatch rule.  
If no compression option is set, then "tgz" is the default.  

## Metadata
It is possible to export the irods avus as a json file included in the bagit archive.  
To enable this feature is necessary to deploy the script available here:  
https://git.ia.surfsara.nl/data-management-services/irods-metadata-translator  

It should be placed in /etc/irods/scripts. It requires both python packages irods-avu-json and python-irodsclient.
The files in the folder "filters" should be placed in /etc/irods. They filter out the keyword "SURFbagit" and "SURFunbagit" from the imported/exported metadata.
Then it possible to export the metadata adding the following option in the usual way:

```
$ imeta set -C /surfTestZone2/home/claudio/mylibrary2 SURFbagit archive copy::zip::avu
```

The metadata are exported recursively for all the sub-collections and objects present under the main collection.  
Pay attention that the position of the three options is important: \[copy|move\]::\[zip|tar|tgz\]::\[avu|acl\]  
By default the avus are exported, while acls must be selected explicitly. However acls are not imported.
The SURFunbagitBatch checks automatically if the exported metadata are available within the bagit package and if it is the case than they are imported back to iRODS.  

## TODO
Performance optimization has not been investigated, in particular about the checksum computation with bdbag.  
