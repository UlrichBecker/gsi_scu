#!/bin/bash
###############################################################################
##                                                                           ##
##       Published the header files and libraries of SCU DAQ and FG          ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:   public.sh                                                         ##
## Author: Ulrich Becker                                                     ##
## (c)     GSI Helmholtz Centre for Heavy Ion Research GmbH                  ##
## Date:   14.10.2020                                                        ##
###############################################################################
ESC_ERROR="\e[1m\e[31m"
ESC_SUCCESS="\e[1m\e[32m"
ESC_NORMAL="\e[0m"

die()
{
   echo -e $ESC_ERROR"ERROR: $@"$ESC_NORMAL 1>&2
   exit 1;
}

if [ ! -n "${OECORE_SDK_VERSION}" ]
then
   if [ "${HOSTNAME:0:5}" = "asl74" ]
   then
      echo "WARNING: YOCTO SDK is not active!" 1>&2
      LIB_SUFFIX="ACC7"
   else
      die "YOCTO SDK is not active!"
   fi
else
   LIB_SUFFIX="Yocto${OECORE_SDK_VERSION}"
fi


if [ ! -n "${OECORE_SDK_VERSION}" ]
then
   CC=gcc
fi

GCC_VERSION=$($CC --version | head -n1 | sed 's/^.* //g' | tr -d . | tr - ' ' | awk '{print $1}')

if [ "$1" = "os" ]
then
   EXPORT_FREE_RTOS_VERSION=true
else
   EXPORT_FREE_RTOS_VERSION=false
fi

SOURCE_BASE_DIR=$(git rev-parse --show-toplevel)/

if [ -n "$DEBUG" ]
then
   DBG_SUFFIX="_dbg"
else
   DBG_SUFFIX=""
fi

LIB_BASE_DIR="prj/scu-control/daq/linux/feedback/"

if [ -n "${OECORE_SDK_VERSION}" ]
then
   LIB_FILE=${SOURCE_BASE_DIR}${LIB_BASE_DIR}deploy_x86_64_sdk_${OECORE_SDK_VERSION}_${GCC_VERSION}${DBG_SUFFIX}/result/libscu_fg_feedback${DBG_SUFFIX}.a
else
   LIB_FILE=${SOURCE_BASE_DIR}${LIB_BASE_DIR}deploy_x86_64_${GCC_VERSION}${DBG_SUFFIX}/result/libscu_fg_feedback${DBG_SUFFIX}.a
fi

if $EXPORT_FREE_RTOS_VERSION
then
   LM32_FW=${SOURCE_BASE_DIR}prj/scu-control/lm32-rtos_exe/SCU3/deploy_lm32/result/scu3_control_os.bin
else
   LM32_FW=${SOURCE_BASE_DIR}prj/scu-control/lm32-non-os_exe/SCU3/deploy_lm32/result/scu3_control.bin
fi

EXAMPLE_FILE=${SOURCE_BASE_DIR}demo-and-test/linux/feedback/feedback-example.cpp

if [ ! -f "Makefile" ]
then
   die "Makefile in \"$PWD\" not found!"
fi

COPY_LIST=$(make ldep | grep "\.h" | grep -v EtherboneConnection\.hpp | grep -v "/etherbone\.h"  | grep -v Constants\.hpp | grep  -v BusException\.hpp)

if [ ! -f "$LIB_FILE" ]
then
   die "Library file: \"$LIB_FILE\" not found!"
fi

if [ ! -f "$LM32_FW" ]
then
  die "LM32 firmware file: \"$LM32_FW\" not found!"
fi

if [ ! -f "$EXAMPLE_FILE" ]
then
   die "Example file \"$EXAMPLE_FILE\" not found"
fi

for i in $COPY_LIST
do
   if [ ! -f "$i" ]
   then
      die "Header file: \"$i\" not found!"
   fi
done

FW_VERSION=$(strings $LM32_FW | grep  "Version     :" | awk '{print $3}')
echo "LM32-Firmware version is: $FW_VERSION"
if [ -n "$(strings $LM32_FW | grep "+MIL-DDR3")" ]
then
   VERSION_DIR=""
   echo "Version will use the DDR3-RAM for MIL-DAQ."
else
   VERSION_DIR="_non_DDR34MIL"
   echo "Version will use the LM32 working memory for MIL-DAQ!"
fi

if [ "${HOSTNAME:0:4}" = "asl7" ]
then
   DESTINATION_BASE_DIR="/common/usr/cscofe/opt/daq-fg/${FW_VERSION}${VERSION_DIR}/"
else
   DESTINATION_BASE_DIR="${HOME}/gsi-export/daq-fg/${FW_VERSION}${VERSION_DIR}/"
fi
HEADER_DIR="${DESTINATION_BASE_DIR}include/"
LIB_DIR="${DESTINATION_BASE_DIR}/lib${LIB_SUFFIX}/"
LM32_BIN_DIR="${DESTINATION_BASE_DIR}lm32_firmware"
EXAMPLE_DIR="${DESTINATION_BASE_DIR}example"

mkdir -p $HEADER_DIR
mkdir -p $LIB_DIR
mkdir -p $LM32_BIN_DIR
mkdir -p $EXAMPLE_DIR

echo -e "Copying \"$(basename $LIB_FILE)\"\\tto \"$LIB_DIR\""
cp -u $LIB_FILE $LIB_DIR

echo -e "Copying \"$(basename $LM32_FW)\"\\tto \"$LM32_BIN_DIR\""
cp -u $LM32_FW $LM32_BIN_DIR

echo -e "Copying \"$(basename $EXAMPLE_FILE)\"\\tto \"$EXAMPLE_DIR\""
cp -u $EXAMPLE_FILE $EXAMPLE_DIR

echo

n=0
for i in $COPY_LIST
do
   echo -e "Copying \"$(basename $i)\"\\tto \"$HEADER_DIR\""
   cp -u $i $HEADER_DIR 2>/dev/null
   [ "$?" == "0" ] && ((n++))
done
echo "============================================"
echo "$n header files copied to $HEADER_DIR"

#=================================== EOF ======================================
