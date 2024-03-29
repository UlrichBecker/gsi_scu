# !/bin/bash
###############################################################################
##                                                                           ##
##          Test script for testing the interface 1 of DDR3-RAM              ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:      ddr3if1test.sh                                                 ##
## Author:    Ulrich Becker                                                  ##
## Date:      24.11.2022                                                     ##
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
Test script for testing the interface 1 of DDR3-RAM
Usage:
  1) on SCU: $(basename $0) [index]
  2) on ASL: $(basename $0) <SCU-URL> [index]

The DDR3 address becomes calculated from the index:
DDR3-address = DDR3-base-address + 8 * index

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
   <description>Test script for testing the interface 1 of DDR3-RAM</description>
   <usage>
      on SCU: on SCU: $(basename $0) [index]
      on ASL: on ASL: $(basename $0) {SCU-URL} [index]
   </usage>
   <author>ubecker@gsi.de</author>
   <tags></tags>
   <version>1.0</version>
   <documentation>
      The DDR3 address becomes calculated from the index:
      DDR3-address = DDR3-base-address + 8 * index
   </documentation>
   <environment>scu</environment>
   <requires>eb-find, eb-read, eb-write</requires>
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

#------------------------------------------------------------------------------
writeReadTest()
{
   let TEST_COUNT+=1
   echo "Test $TEST_COUNT:"
   echo "Writing on index: $INDEX, address: $TEST_ADDR, pattern: $1"
   eb-write -q $TARGET $TEST_ADDR/$DDR3_ALIGN $1

   local readBack="0x$(toupper $(eb-read -q $TARGET $TEST_ADDR/$DDR3_ALIGN))"
   echo " Reding on index: $INDEX, address: $TEST_ADDR, pattern: $readBack"

   if [ "$1" = "$readBack" ]
   then
      echo -e $ESC_SUCCESS"*** Pass! :-) ***"$ESC_NORMAL
      let PASS_COUNT+=1
   else
      echo -e $ESC_ERROR"*** Fail! :-( ***"$ESC_NORMAL
      let FAIL_COUNT+=1
   fi
   echo
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
   checkTarget $1
   TARGET="tcp/$1"
   shift
else
   TARGET="dev/wbm0"
fi

if [ ! -n "$1" ]
then
   INDEX=0
else
   INDEX=$1
fi

if [ ! -n "$(which eb-find 2>/dev/null)" ]
then
   die "\"eb-find\" not found!"
fi

if [ ! -n "$(which eb-read 2>/dev/null)" ]
then
   die "\"eb-read\" not found!"
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

echo "Base address of DDR3 interface 1 is: $IF1_BASE_ADDR"

let OFFSET=INDEX*DDR3_ALIGN

TEST_ADDR="$(printf "0x%X\n" $((IF1_BASE_ADDR+OFFSET)))"

VALUE=$(eb-read -q $TARGET $TEST_ADDR/$DDR3_ALIGN)
echo "DDR3[$INDEX] addr: $TEST_ADDR: 0x$VALUE"

echo "Start of write/read test..."

writeReadTest 0x1122334455667788
writeReadTest 0x0000000000000000
writeReadTest 0xFFFFFFFFFFFFFFFF
writeReadTest 0xAAAAAAAAAAAAAAAA
writeReadTest 0x5555555555555555

echo "Summary:"
echo "Failed: $FAIL_COUNT of $TEST_COUNT"
echo "Passed: $PASS_COUNT of $TEST_COUNT"
echo "================"

#=================================== EOF ======================================
