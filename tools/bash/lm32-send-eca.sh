# !/bin/bash
###############################################################################
##                                                                           ##
##             Script sends a ECA-event to a LM32- application               ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:      lm32-send-eca.sh                                               ##
## Author:    Ulrich Becker                                                  ##
## Date:      24.06.2024                                                     ##
## Copyright: GSI Helmholtz Centre for Heavy Ion Research GmbH               ##
###############################################################################
PROG_NAME=${0##*/}
ESC_ERROR="\e[1m\e[31m"
ESC_SUCCESS="\e[1m\e[32m"
ESC_NORMAL="\e[0m"
ESC_FG_CYAN="\e[1m\e[36m"
ESC_FG_BLUE="\e[34m"

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
   Sends a ECA to the LM32- application.
   NOTE: This is for develop and debug porposes only!

   Usage: $PROG_NAME <ECA-event number>
__EOH__
   exit 0
}

#==============================================================================

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
            A=""
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

if [ ! -n "$1" ]
then
   die "Missing ECA number!"
fi

LM32_CTL=$(which saft-ecpu-ctl)
if [ ! -x "$LM32_CTL" ]
then
   die "Program saft-ecpu-ctl not found!"
fi

SAFT_CTL=$(which saft-ctl)
if [ ! -x "$SAFT_CTL" ]
then
   die "Program saft-ctl not found!"
fi

OUT="tr0"
LM32_CTL="$LM32_CTL $OUT"
SAFT_CTL="$SAFT_CTL $OUT"

$LM32_CTL -x
$LM32_CTL -c 0xCAFEBABE 0xFFFFFFFFFFFFFFFF 0x0 $1 -d
$SAFT_CTL inject 0xCAFEBABE 0xFFFFFFFFFFFFFFFF 10000 -p
$LM32_CTL -x
echo "done"
#=================================== EOF ======================================
