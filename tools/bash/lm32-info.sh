#!/bin/sh
###############################################################################
##                                                                           ##
##    Shellscript to display the build-ID of the LM32-firmware in a SCU      ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:      lm32-info.sh                                                   ##
## Author:    Ulrich Becker                                                  ##
## Date:      20.11.2020                                                     ##
## Copyright: GSI Helmholtz Centre for Heavy Ion Research GmbH               ##
###############################################################################
ESC_ERROR="\e[1m\e[31m"
ESC_NORMAL="\e[0m"

DEV=dev/wbm0
GSI_POSTFIX=".acc.gsi.de"

die()
{
   echo -e $ESC_ERROR"ERROR: $@"$ESC_NORMAL 1>&2
   exit 1;
}

checkTarget()
{
   [ ! -n "$1" ] && die "Missing target URL!"
   ping -c1 $1 2>/dev/null 1>/dev/null
   [ "$?" != '0' ] && die "Target \"$1\" not found!"
}

if [ "$1" == "-h" ]
then
   echo "Tool to display the build-id of the currently running LM32-firmware" \
        "in a SCU"
   echo "Usage $0 <URL of target SCU>"
   exit 0
fi

SCU_NAME=$1
if [ ! -n "$(echo $SCU_NAME | grep $GSI_POSTFIX )" ]
then
   SCU_NAME=${SCU_NAME}${GSI_POSTFIX}
fi

if [ "${HOSTNAME:0:4}" = "asl7" ] || $IN_GSI_NET
then
   #
   # Script is running on ASL-cluster
   #
   checkTarget $SCU_NAME
   if [ -n "$(which lm32-logd 2>/dev/null)" ]
   then
      lm32-logd -B $SCU_NAME
   else
      echo "Calling eb-info on $SCU_NAME:"
      echo "------------------------"
      ssh root@${SCU_NAME} "eb-info -w $DEV"
   fi
   exit $?
else
   #
   # Script ts running on remote-PC
   #
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
   ssh -t ${GSI_USERNAME}@${ASL_URL} $(basename $0) $SCU_NAME
fi

#=================================== EOF ======================================
