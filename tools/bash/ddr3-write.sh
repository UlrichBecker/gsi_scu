# !/bin/bash
###############################################################################
##                                                                           ##
##              Writes a 64-bit value in the SCU-DDR3-RAM                    ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:      ddr3-write.sh                                                  ##
## Author:    Ulrich Becker                                                  ##
## Date:      28.11.2022                                                     ##
## Copyright: GSI Helmholtz Centre for Heavy Ion Research GmbH               ##
###############################################################################
ESC_ERROR="\e[1m\e[31m"
ESC_SUCCESS="\e[1m\e[32m"
ESC_NORMAL="\e[0m"

DDR3_ALIGN=8
FAIL_COUNT=0
PASS_COUNT=0
TEST_COUNT=0

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
Writes a 64-bit value in the SCU-DDR3-RAM
Usage:
  1) on SCU: $(basename $0) [option] <index> <64-bit value>
  2) on ASL: $(basename $0) [option] <SCU-URL> <index> <64-bit value>

The DDR3 address becomes calculated from the index:
DDR3-address = DDR3-base-address + 8 * index

Options:
-h, --help
   This help and exit.

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
<description>Writes a 64-bit value in the SCU-DDR3-RAM</description>
<usage>
on SCU: $(basename $0) [option] <index> <64-bit value>
on ASL: $(basename $0) [option] <SCU-URL> <index> <64-bit value>
</usage>
<author>ubecker@gsi.de</author>
<tags></tags>
<version>1.0</version>
<documentation>
The DDR3 address becomes calculated from the index:
DDR3-address = DDR3-base-address + 8 * index
</documentation>
<environment>scu</environment>
<requires>eb-find, eb-write</requires>
<autodocversion>1.0</autodocversion>
<compatibility></compatibility>
</toolinfo>"
__EOH__
   exit 0
}

#------------------------------------------------------------------------------
toupper()
{
   printf '%s\n' "$1" | awk '{ print toupper($0) }'
}

#==============================================================================
BE_VERBOSE=false
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

if [ "$#" -ne 2 ]
then
   die "Missing argument!"
fi

INDEX=$1
VALUE=$2

if [ ! -n "$(which eb-find 2>/dev/null)" ]
then
   die "\"eb-find\" not found!"
fi

if [ ! -n "$(which eb-write 2>/dev/null)" ]
then
   die "\"eb-write\" not found!"
fi

IF1_BASE_ADDR=$(eb-find $TARGET 0x651 0x20150828)
if [ "$?" -ne "0" ]
then
   die "Can't find base address of DDR3 interface 1"
fi

if $BE_VERBOSE
then
   echo "Base address of DDR3 interface 1 is: $IF1_BASE_ADDR"
fi

let OFFSET=INDEX*DDR3_ALIGN

DDR3_ADDR="$(printf "0x%X\n" $((IF1_BASE_ADDR+OFFSET)))"

if $BE_VERBOSE
then
   echo "Address: $DDR3_ADDR"
else
   EB_VERBOSE="-q"
fi

eb-write $EB_VERBOSE $TARGET $DDR3_ADDR/$DDR3_ALIGN $VALUE

#=================================== EOF ======================================
