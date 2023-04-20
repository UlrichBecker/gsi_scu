# !/bin/bash
###############################################################################
##                                                                           ##
##  Wrapper helper script for most popular using of application fg-feedback  ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:      feedback.sh                                                    ##
## Author:    Ulrich Becker                                                  ##
## Date:      20.04.2023                                                     ##
## Copyright: GSI Helmholtz Centre for Heavy Ion Research GmbH               ##
###############################################################################
ESC_ERROR="\e[1m\e[31m"
ESC_SUCCESS="\e[1m\e[32m"
ESC_NORMAL="\e[0m"

#------------------------------------------------------------------------------
die()
{
   echo -e $ESC_ERROR"ERROR: $@"$ESC_NORMAL 1>&2
   exit 1;
}

#------------------------------------------------------------------------------
checkTarget()
{
   [ ! -n "$1" ] && die "Missing target URL!"
   ping -c1 $1 2>/dev/null 1>/dev/null
   [ "$?" != '0' ] && die "Target \"$1\" not found!"
}

#------------------------------------------------------------------------------
printHelp()
{
   cat << __EOH__
Wrapper helper script for most popular using of application fg-feedback
Usage:
  $(basename $0) [SCU-name (on ASL only)]


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
   checkTarget $1
   TARGET=$1
   shift
else
   TARGET=""
fi

if [ ! -x "$(which fg-feedback 2>/dev/null)" ]
then
   die "Application \"fg-feedback\" not found!"
fi

fg-feedback $TARGET -azt1 -I200 -T"X11 size 800,150"  -u0

#=================================== EOF ======================================
