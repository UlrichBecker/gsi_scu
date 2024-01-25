# !/bin/bash
###############################################################################
##                                                                           ##
##                Wrapper script for "rsync" with "ssh"                      ##
##                for remote synchronization of folders.                     ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:       syncit.sh                                                     ##
## Author:     Ulrich Becker                                                 ##
## Date:       25.01.2024                                                    ##
## Copyright:  GSI Helmholtz Centre for Heavy Ion Research GmbH              ##
###############################################################################
ESC_ERROR="\e[1m\e[31m"
ESC_SUCCESS="\e[1m\e[32m"
ESC_NORMAL="\e[0m"

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

printHelp()
{
   echo "Synchronizes a local directory with a remote directory of GSI-ASL-Cluster"
   echo "usage: $0 [options] <local directory> <remote directory>"
   exit 0
}

while [ "${1:0:1}" = "-" ]
do
   A=${1#-}
   while [ -n "$A" ]
   do
      case ${A:0:1} in
         "h")
            printHelp
         ;;
         "d")
            MAKE_DELETE="--delete"
         ;;
         "-")
            B=${A#*-}
            case ${B%=*} in
               "help")
                  printHelp
               ;;
               "delete")
                  MAKE_DELETE="--delete"
               ;;
               *)
                  die "Unknown long option \"-${A}\"!"
               ;;
            esac
         ;;
         *)
            die "Unknown short option: \"${A:0:1}\"!"
         ;;
      esac
      A=${A#?}
   done
   shift
done

echo "Synchronize it..."

if [ "${HOSTNAME:0:5}" = "asl75" ]
then
   die "You are on the ${HOSTNAME}. Script works only on remote computer!"
fi

LOCAL_SYNC_DIR=$1
REMOTE_SYNC_DIR=$2

if [ ! -n "$LOCAL_SYNC_DIR" ]
then
   die "Missing arguments!"
fi

REMOTE_URL=asl751.acc.gsi.de

if [ ! -d "$LOCAL_SYNC_DIR" ]
then
   die "Home folder: \"$LOCAL_SYNC_DIR\" not found!"
fi
if [ ! -n "$REMOTE_SYNC_DIR" ]
then
   die "Missing remote folder!"
fi

checkTarget $REMOTE_URL

if [ ! -n "${GSI_USERNAME}" ]
then
   die "Missing environment variable \"GSI_USERNAME\""
fi

ssh -t ${GSI_USERNAME}@${REMOTE_URL} "test -d ${REMOTE_SYNC_DIR}" 2>/dev/null 1>/dev/null
if [ "$?" != "0" ]
then
   die "Remote folder: \"${GSI_USERNAME}@${REMOTE_URL}:${REMOTE_SYNC_DIR}\" not found!"
fi

rsync -av $MAKE_DELETE -e ssh ${GSI_USERNAME}@${REMOTE_URL}:${REMOTE_SYNC_DIR} $LOCAL_SYNC_DIR
if [ "$?" != "0" ]
then
   die "rsync!"
fi

echo -e "${ESC_SUCCESS}done${ESC_NORMAL}"
#=================================== EOF ======================================
