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

if [ ! -n "${OECORE_SDK_VERSION}" ]
then
   echo "WARNING: YOCTO SDK is not active!" 1>&2
fi

GCC_VERSION=$(gcc --version | head -n1 | sed 's/^.* //g' | tr -d . | tr - ' ' | awk '{print $1}')

if [ "$1" = "os" ]
then
   EXPORT_FREE_RTOS_VERSION=true
else
   EXPORT_FREE_RTOS_VERSION=false
fi

SOURCE_BASE_DIR=$(git rev-parse --show-toplevel)

if [ -n "${OECORE_SDK_VERSION}" ]
then
   LIB_FILE=${SOURCE_BASE_DIR}/prj/scu-control/daq/linux/feedback/deploy_x86_64_${GCC_VERSION}_sdk_${OECORE_SDK_VERSION}/result/libscu_fg_feedback.a
else
   LIB_FILE=${SOURCE_BASE_DIR}/prj/scu-control/daq/linux/feedback/deploy_x86_64_${GCC_VERSION}/result/libscu_fg_feedback.a
fi

if $EXPORT_FREE_RTOS_VERSION
then
   LM32_FW=${SOURCE_BASE_DIR}/prj/scu-control/lm32-rtos_exe/SCU3/deploy_lm32/result/scu3_control_os.bin
else
   LM32_FW=${SOURCE_BASE_DIR}/prj/scu-control/lm32-non-os_exe/SCU3/deploy_lm32/result/scu3_control.bin
fi

EXAMPLE_FILE=${SOURCE_BASE_DIR}/demo-and-test/linux/feedback/feedback-example.cpp


COPY_LIST=$(make ldep | grep "\.h" | grep -v EtherboneConnection\.hpp | grep -v "/etherbone\.h"  | grep -v Constants\.hpp )

if [ ! -f "$LIB_FILE" ]
then
   echo "Library file: \"$LIB_FILE\" not found!" 1>&2
   exit 1
fi

if [ ! -f "$LM32_FW" ]
then
  echo "LM32 firmware file: \"$LM32_FW\" not found!" 1>&2
  exit 1
fi

if [ ! -f "$EXAMPLE_FILE" ]
then
   echo "Example file \"$EXAMPLE_FILE\" not found" 1>&2
   exit 1
fi

for i in $COPY_LIST
do
   if [ ! -f "$i" ]
   then
      echo "Header file: \"$i\" not found!" 1>&2
      exit 1
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
LIB_DIR="${DESTINATION_BASE_DIR}/lib/"
LM32_BIN_DIR="${DESTINATION_BASE_DIR}lm32_firmware"
EXAMPLE_DIR="${DESTINATION_BASE_DIR}example"

mkdir -p $HEADER_DIR
mkdir -p $LIB_DIR
mkdir -p $LM32_BIN_DIR
mkdir -p $EXAMPLE_DIR
cp -u $LIB_FILE $LIB_DIR
cp -u $LM32_FW $LM32_BIN_DIR
cp -u $EXAMPLE_FILE $EXAMPLE_DIR

n=0
for i in $COPY_LIST
do
   cp -u $i $HEADER_DIR 2>/dev/null
   [ "$?" == "0" ] && ((n++))
done

echo "$n header files copied to $HEADER_DIR"

#=================================== EOF ======================================
