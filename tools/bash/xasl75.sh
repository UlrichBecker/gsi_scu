# !/bin/bash
###############################################################################
##                                                                           ##
##     Shellscript for establishing a X-connection via ssh and xfreerdp      ##
##     to ASL751 til ASL755.                                                 ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:      xasl75.sh                                                      ##
## Author:    Ulrich Becker                                                  ##
## Date:      29.08.2022                                                     ##
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


if [ ! -n "$1" ]
then
   die "Missing argument (last digit of ASL-number): 1, 2, 3, 4 or 5 !"
fi
if [ "$1" -lt "1" ] || [ "$1" -gt "5" ] 
then
   die "ASL-number $1 out of range!"
fi

SERVER="asl75$1.acc.gsi.de"
LOCAL_PORT=1${SERVER:3:3}

if [ ! -n "$GSI_USERNAME" ]
then
   echo "GSI username: "
   read GSI_USERNAME
fi

LC_ALL=C 
#
# 2>&1 doesn't work with ssh. I dont know why yet. :-/
#
TMP_FILE=/tmp/${SERVER}.tmp
ssh $GSI_USERNAME@$SERVER -L $LOCAL_PORT:localhost:3389 -N -f 2>$TMP_FILE
if [ "?" != "0" ]
then
   if [ ! -n "$(cat $TMP_FILE | grep "Address already in use")" ]
   then
      ERROR_TEXT=$(cat $TMP_FILE)
      rm $TMP_FILE
      die "$ERROR_TEXT"
   else
      echo "INFO: Connection to $SERVER:$LOCAL_PORT already established."
   fi
else
   echo "INFO: First connection to $SERVER:$LOCAL_PORT ."
   sleep 1
fi
rm $TMP_FILE

xfreerdp +clipboard /u:$GSI_USERNAME /f +fonts +glyph-cache /v:localhost:$LOCAL_PORT /relax-order-checks /dynamic-resolution 

#=================================== EOF ======================================
