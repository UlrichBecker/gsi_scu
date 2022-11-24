# !/bin/bash 
###############################################################################
##                                                                           ##
##              Shows the lm32 logfile updated by lm32-logd                  ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:      slogf.sh                                                       ##
## Author:    Ulrich Becker                                                  ##
## Date:      24.11.2022                                                     ##
## Copyright: GSI Helmholtz Centre for Heavy Ion Research GmbH               ##
###############################################################################
ESC_ERROR="\e[1m\e[31m"
ESC_NORMAL="\e[0m"

LOGFILE=/var/log/lm32.log

#------------------------------------------------------------------------------
die()
{
   echo -e $ESC_ERROR"ERROR: $@"$ESC_NORMAL 1>&2
   exit 1;
}

#------------------------------------------------------------------------------
printHelp()
{
   cat << __EOH__
Script schows the lm32 logfile when it was updated by lm32-logd!

Options:
-h, --help 
   This help and exit.

__EOH__
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
         "-")
            B=${A#*-}
            case ${B%=*} in
               "help")
                  printHelp
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

if [ "${HOSTNAME:0:5}" != "scuxl" ]
then
   die "This script can run on SCU only!"
fi

if [ ! -f "$LOGFILE" ]
then
   die "Can't find the logfile \"$LOGFILE\"!" 
fi

tail -n100 -f $LOGFILE

#=================================== EOF ======================================