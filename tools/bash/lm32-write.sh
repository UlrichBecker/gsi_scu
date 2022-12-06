# !/bin/bash
###############################################################################
##                                                                           ##
##              Writes a 32-bit value in the LM32-user-RAM                   ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:      lm32-write.sh                                                  ##
## Author:    Ulrich Becker                                                  ##
## Date:      06.12.2022                                                     ##
## Copyright: GSI Helmholtz Centre for Heavy Ion Research GmbH               ##
###############################################################################
ESC_ERROR="\e[1m\e[31m"
ESC_SUCCESS="\e[1m\e[32m"
ESC_NORMAL="\e[0m"

LM32_ALIGN=4

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
Writes a 32-bit value in the LM32 user RAM.
  1) on SCU: $(basename $0) [options] <relative address> <value>
  2) on ASL: $(basename $0) [options] <SCU-URL> <relative address> <value>

Options:
-h, --help
   This help and exit.

-n <index>
   CPU number (default is 0)

-v, --verbose
   Be verbose

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
   <description>Writes a 32-bit value in the LM32 user RAM</description>
   <usage>
      on SCU: $(basename $0) [options] {relative address} {value}
      on ASL: $(basename $0) [options] {SCU-URL} {relative address} {value}
   </usage>
   <author>ubecker@gsi.de</author>
   <tags></tags>
   <version>1.0</version>
   <documentation></documentation>
   <requires>eb-find, eb-write</requires>
   <autodocversion>1.0</autodocversion>
   <compatibility></compatibility>
</toolinfo>"
__EOH__
   exit 0
}

#==============================================================================
BE_VERBOSE=false
CPU_NO=0
while [ "${1:0:1}" = "-" ]
do
   A=${1#-}
   while [ -n "$A" ]
   do
      case ${A:0:1} in
         "h")
            printHelp
         ;;
         "v")
            BE_VERBOSE=true
         ;;
         "n")
            if  [ ! -n "${A:1:1}" ]
            then
               shift
               CPU_NO=$1
            else
               CPU_NO=${A:1:1}
               A=""
            fi
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
               "verbose")
                  BE_VERBOSE=true
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
   TARGET="tcp/$1"
   shift
else
   TARGET="dev/wbm0"
fi

if [ ! -n "$1" ]
then
   die "Missing relative offset address!"
fi

if [ ! -n "$2" ]
then
   die "Missing 32-bit value to write!"
fi

if [ ! -n "$(which eb-find 2>/dev/null)" ]
then
   die "\"eb-find\" not found!"
fi

if [ ! -n "$(which eb-write 2>/dev/null)" ]
then
   die "\"eb-write\" not found!"
fi

LM32_BASE_ADDR=$(eb-find -n $CPU_NO $TARGET 0x651 0x54111351)
if [ "$?" -ne "0" ]
then
   die "Can't find base address of LM32[$CPU_NO]"
fi

let LM32_ADDRESS=LM32_BASE_ADDR+$1

if $BE_VERBOSE
then
   echo "Base address of lm32 is: $LM32_BASE_ADDR"
   printf "Address to read is:      0x%X\n"  $LM32_ADDRESS
fi

eb-write $TARGET $LM32_ADDRESS/$LM32_ALIGN $2

#=================================== EOF ======================================
