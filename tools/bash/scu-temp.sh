# !/bin/bash
###############################################################################
##                                                                           ##
##  Reads board temperature, backplane temperature and extern temperature    ##
##     from the shared memory of the LM32-application scu_control_os         ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:      scu-temp.sh                                                    ##
## Author:    Ulrich Becker                                                  ##
## Date:      29.11.2022                                                     ##
## Copyright: GSI Helmholtz Centre for Heavy Ion Research GmbH               ##
###############################################################################
ESC_ERROR="\e[1m\e[31m"
ESC_SUCCESS="\e[1m\e[32m"
ESC_NORMAL="\e[0m"

ADDR_BOARD_TEMP=0x518
ADDR_BACKPLANE_TEMP=0x520
ADDR_EXTERN_TEMP=0x51C
ADDR_MAGIC_NUMBER=0x524
MAGIC_NUMBER=DEADBEEF

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
Reads board temperature, backplane temperature and extern temperature
from the shared memory of the LM32-application scu_control_os

Usage:
  1) on SCU: $(basename $0)
  2) on ASL: $(basename $0) <SCU-URL>

NOTE:
  Really actual temperatures are only obtainable when the FreeRTOS application
  "scu_control_os" is running!

Options:
-h, --help
   This help and exit.

__EOH__
   exit 0
}

#------------------------------------------------------------------------------
docTagged()
{
   cat << __EOH__
<toolinfo>
   <name>$(basename $0)</name>
   <topic>Helpers</topic>
   <description>
      Reads board temperature, backplane temperature and extern temperature
      from the shared memory of the LM32-application scu_control_os
   </description>
   <usage>
      on SCU: $(basename $0)
      on ASL: $(basename $0) {SCU-URL}
   </usage>
   <author>ubecker@gsi.de</author>
   <tags></tags>
   <version>1.0</version>
   <documentation>
   </documentation>
   <environment>scu</environment>
   <requires>lm32-read</requires>
   <autodocversion>1.0</autodocversion>
   <compatibility></compatibility>
</toolinfo>"
__EOH__
   exit 0
}

#------------------------------------------------------------------------------
printTemperature()
{
   local temp="0x$(lm32-read.sh $1 $2)"
   if [ "$temp" = "0xFFFFFFFF" ]
   then
      echo -e $ESC_ERROR"failed!"$ESC_NORMAL
   else
      printf "%d.%u °C\n" $((temp/16)) $(((temp&0x0F)*10/16))
   fi
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
               "generate_doc_tagged")
                  docTagged
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
   if [ ! -n "$1" ]
   then
      die "Missing SCU-URL"
   fi
   TARGET=$1
else
   TARGET=""
fi

if [ ! -n "$(which lm32-read.sh 2>/dev/null)" ]
then
   die "\"lm32-read\" not found!"
fi

if [ "$(lm32-read.sh $1 $ADDR_MAGIC_NUMBER)" != "$MAGIC_NUMBER" ]
then
   die "Magic number not found!"
fi

echo "Extern:    $(printTemperature $TARGET $ADDR_EXTERN_TEMP)"
echo "Backplane: $(printTemperature $TARGET $ADDR_BACKPLANE_TEMP)"
echo "Board:     $(printTemperature $TARGET $ADDR_BOARD_TEMP)"

#=================================== EOF ======================================
