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
   TARGET="tcp/$1"
   shift
else
   TARGET="dev/wbm0"
fi

if [ !-n "$1" ]
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

TEST_PATTREN=0x1122334455667788
#TEST_PATTREN=0x0000000000000000

echo "Writing on index $INDEX pattern $TEST_PATTREN"
eb-write -q $TARGET $TEST_ADDR/$DDR3_ALIGN $TEST_PATTREN

VALUE=$(eb-read -q $TARGET $TEST_ADDR/$DDR3_ALIGN)
echo "DDR3[$INDEX] addr:  $TEST_ADDR: 0x$VALUE"

if [ "$TEST_PATTREN" = "$VALUE" ]
then
   echo -e $ESC_SUCCESS"*** Pass! :-) ***"$ESC_NORMAL
else
   echo -e $ESC_ERROR"*** Fail! :-( ***"$ESC_NORMAL
fi


#=================================== EOF ======================================
