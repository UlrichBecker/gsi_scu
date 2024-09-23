# !/bin/bash
###############################################################################
##                                                                           ##
##          Shows the number of running threads of a given process           ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:   threadsof.sh                                                      ##
## Author: Ulrich Becker                                                     ##
## Date:   32.09.2024                                                        ##
## Copyright: GSI Helmholtz Centre for Heavy Ion Research GmbH               ##
###############################################################################
ESC_ERROR="\e[1m\e[31m"
ESC_NORMAL="\e[0m"

if [ "$1" = "-h" ]
then
   echo "Lists the number of threads in the currently running processes of the given name."
   exit 0
fi

if [ ! -n "$1" ]
then
   echo -e $ESC_ERROR"ERROR: Missing argument"$ESC_NORMAL 1>&2
   exit 1
fi

PROC_NAME=$1

if [ ! -n "$(ps -e | awk {'print $4'} | grep $PROC_NAME)" ]
then
   echo -e $ESC_ERROR"ERROR: No running proces(es) with name \"$PROC_NAME\" found!"$ESC_NORMAL 1>&2
   exit 1
fi

PROC_IDS=$(ps -e | grep $PROC_NAME | grep -v grep | grep -v $(basename $0) | awk '{print $1}')

if [ ! -n "$PROC_IDS" ]
then
   echo -e $ESC_ERROR"ERROR: Could not found any running thread!"$ESC_NORMAL 1>&2
   exit -1
fi

for i in $PROC_IDS
do
   if [ -d "/proc/$i" ]
   then
      numOfThreads=$(cat /proc/$i/status | grep Threads | awk '{print $2}')
      echo -e "PID: $i,\t threads: $numOfThreads"
   fi
done
exit 0
#=================================== EOF ======================================
