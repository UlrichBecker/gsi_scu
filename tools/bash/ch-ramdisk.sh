# !/bin/bash
###############################################################################
##                                                                           ##
##     Skript chances the RAMDISK from "scuxl" to "yocto" and vice versa.    ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:      ch-ramdisk.sh                                                  ##
## Author:    Ulrich Becker                                                  ##
## Date:      15.04.2021                                                     ##
## Copyright: GSI Helmholtz Centre for Heavy Ion Research GmbH               ##
###############################################################################
ESC_ERROR="\e[1m\e[31m"
ESC_SUCCESS="\e[1m\e[32m"
ESC_NORMAL="\e[0m"

die()
{
   echo -e $ESC_ERROR"ERROR: $@"$ESC_NORMAL 1>&2
   exit 1;
}

if [ "${HOSTNAME:0:5}" = "asl74" ]
then
   die "Deprecated host!"
fi

if [ "${HOSTNAME:0:5}" = "asl75" ]
then
   #
   # Script is running on ASL-cluster
   #
   if [ "$1" = "-i" ]
   then
      RD_NAME=$1
      SCU_NAME=$2
   else
      RD_NAME=$2
      SCU_NAME=$1
   fi
   RAMDISK_LNK_DIR=/common/tftp/csco/pxe/pxelinux.cfg
   if [ ! -d "$RAMDISK_LNK_DIR" ]
   then
      die "Folder \"$RAMDISK_LNK_DIR\" not found!"
   fi
   NFS_INIT_DIR=/common/export/nfsinit/$SCU_NAME
   if [ ! -d "$NFS_INIT_DIR" ]
   then
      die "Folder \"$NFS_INIT_DIR\" not found!"
   fi
   SCU=$RAMDISK_LNK_DIR/$SCU_NAME
   if [ ! -L "$SCU" ]
   then
      die "SCU \"$SCU_NAME\" not found!"
   fi
   if [ "$RD_NAME" = "-i" ]
   then
      basename $(readlink $SCU)
      exit 0
   fi

   RAMDISK=$RAMDISK_LNK_DIR/$RD_NAME
   if [ ! -f "$RAMDISK" ]
   then
      die "RAM-disc file \"$RD_NAME\" not found"
   fi
   if [ "$(basename $(readlink $SCU))" = "$RD_NAME" ]
   then
      echo "Nothing to do..."
      exit 0
   fi

   echo "Exchanging current RAMDISK \"$(basename $(readlink $SCU))\" to \"$RD_NAME\"."
   unlink $SCU
   if [ "$?" != "0" ]
   then
      die "Can't unlink \"$SCU\" !"
   fi

   ln -s $RAMDISK $SCU
   if [ "$?" != "0" ]
   then
      ln -s $SCU $RAMDISK
      die "Can't make symbolic link \"$RAMDISK\" -> \"$SCU\" !"
   fi

   #TODO!
  # exit 0
  # if [ ! -f "$NFS_INIT_DIR/50_fesa_dev" ]
  # then
  #    die "Symbolic link \"$NFS_INIT_DIR/50_fesa_dev\" not found!"
  # fi
   echo "done"
else
   #
   # Script is running on remote-PC
   #
   GSI_POSTFIX=".acc.gsi.de"
   if [ ! -n "$GSI_USERNAME" ]
   then
      echo "GSI username: "
      read GSI_USERNAME
   fi
   if [ ! -n "$ASL_NO" ]
   then
      ASL_NO=755
   fi
   ASL_URL=asl${ASL_NO}${GSI_POSTFIX}
   echo "Recursive call on asl${ASL_NO}:"
   ssh -t ${GSI_USERNAME}@${ASL_URL} $(basename $0) $1 $2
fi

#=================================== EOF ======================================
