# !/bin/bash
###############################################################################
##                                                                           ##
##               Script MSI-events in a loop for test purposes               ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:      make-msi.sh                                                    ##
## Author:    Ulrich Becker                                                  ##
## Date:      09.11.2022                                                     ##
## Copyright: GSI Helmholtz Centre for Heavy Ion Research GmbH               ##
###############################################################################
ESC_ERROR="\e[1m\e[31m"
ESC_SUCCESS="\e[1m\e[32m"
ESC_NORMAL="\e[0m"

DEV=tr0
ID=0x1122334455667788
MASK=0xFFFFFFFFFFFFFFFF
OFFSET=0

#------------------------------------------------------------------------------
die()
{
   echo -e $ESC_ERROR"ERROR: $@"$ESC_NORMAL 1>&2
   exit 1;
}

#if  [ ! -f /dev/wbm0 ]
#then
#   die "Script doesn't start on SCU!"
#fi

if [ ! -x "$(which saft-ecpu-ctl)" ]
then
   die "App saft-ecpu-ctl not found!"
fi

if [ ! -x "$(which saft-ctl)" ]
then
   die "App saft-ctl not found!"
fi

saft-ecpu-ctl $DEV -x

onExit()
{
   saft-ecpu-ctl $DEV -x
   echo "done"
   exit 0;
}

trap onExit SIGINT SIGTERM

if [ -n "$2" ]
then
   PARAM=$2
else
   PARAM=0x8877887766556642
fi

i=0
while [ $i -lt $1 ]
do
  saft-ecpu-ctl $DEV -d -c $ID $MASK $OFFSET 0x42
  let i+=1
done

echo "Type ctl+C to end, $$"
while true
do
   saft-ctl $DEV inject $ID $PARAM 0
done

#=================================== EOF ======================================
