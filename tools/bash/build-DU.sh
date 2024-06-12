###############################################################################
##                                                                           ##
##      Build-base script for developing and testing of a FESA class         ##
##                                                                           ##
## Note this script schall not be invoked directly, it schall be included in ##
## a for one FESA-class specialized shellscript.                             ##
###############################################################################
## File:   build-DU.sh                                                       ##
## Author: Ulrich Becker                                                     ##
## (C)     GSI Helmholtz Centre for Heavy Ion Research GmbH                  ##
## Date:   09.04.2024                                                        ##
###############################################################################
#   In your individual build-script the three following variables has to be   #
#   initialized like the example below before this file becomes included:     #
#                                                                             #
# #!/bin/bash                                                                 #
# TARGET=scuxl4711                                                            #
# FESA_CLASS_NAME=MyFesaClass                                                 #
# DU_NAME=MyDeployUnit                                                        #
# source /common/usr/cscofe/scripts/build-DU.sh                               #
###############################################################################
ESC_ERROR="\e[1m\e[31m"
ESC_SUCCESS="\e[1m\e[32m"
ESC_NORMAL="\e[0m"
ORIGIN_DIR=$(pwd)

#------------------------------------------------------------------------------
die()
{
   echo -e $ESC_ERROR"ERROR: $@"$ESC_NORMAL 1>&2
   cd $ORIGIN_DIR
   exit 1;
}

if [ "${HOSTNAME:0:6}" != "asl745" ]
then
   if [ ! -n "$OECORE_SDK_VERSION" ]
   then
      if [ "${HOSTNAME:0:5}" = "asl75" ]
      then
         YOKTO_SDK_PATH=/common/usr/embedded/yocto/fesa/current/sdk/
      else
         if [ ! -n "$YOKTO_SDK_PATH" ]
         then
            die "YOKTO_SDK_PATH is not defined!"
         fi
      fi
      if [ ! -d "$YOKTO_SDK_PATH" ]
      then
         die "Directory for Yocto-SDK: \"$YOKTO_SDK_PATH\" not found!"
      fi
      if [ ! -n "$SDK_SETUP_FILE" ]
      then
         SDK_SETUP_FILE="environment-setup-core2-64-ffos-linux"
      fi
      if [ ! -f "${YOKTO_SDK_PATH}${SDK_SETUP_FILE}" ]
      then
         die "SDK- setup file \"${YOKTO_SDK_PATH}${SDK_SETUP_FILE}\" for Yocto not found!"
      fi
      unset $LD_LIBRARY_PATH
      source ${YOKTO_SDK_PATH}${SDK_SETUP_FILE}
   fi
   echo "Build by Yocto-SDK version: \"$OECORE_SDK_VERSION\""
else
   echo "Build on ACC7."
fi

if [ "$1" = "clean" ]
then
   MAKE_ARG="clean"
fi

if [ ! -n "$TARGET" ]
then
   die "Variable \"TARGET\" is not defined!"
fi

if [ ! -n "$FESA_CLASS_NAME" ]
then
   die "Variable \"FESA_CLASS_NAME\" is not defined!"
fi

if [ ! -n "$DU_NAME" ]
then
   die "Variable \"DU_NAME\" is not defined!"
fi

if [ ! -d "$DU_NAME" ]
then
   die "Directory \"${DU_NAME}\" not found!"
fi

if [ ! -d "${FESA_CLASS_NAME}" ]
then
   die "Drectory \"${FESA_CLASS_NAME}\" not found!"
fi

if [ ! -f "${FESA_CLASS_NAME}/Makefile" ]
then
   die "Makefile for \"${FESA_CLASS_NAME}\" not found!"
fi

if [ ! -f "${DU_NAME}/Makefile" ]
then
   die "Makefile for \"${DU_NAME}\" not found!"
fi

if [ ! -n "$EXPORT_DIR" ]
then
   EXPORT_DIR=/common/export/fesa/local/${TARGET}
fi

if [ ! -d "$EXPORT_DIR" ]
then
   die "Export directory \"$EXPORT_DIR\" not found!"
fi

if [ ! -n "$BINARY_SUFFIX" ]
then
   BINARY_SUFFIX=_M
fi

if [ ! -n "$BINARY_DIR_SUFFIX" ]
then
   BINARY_DIR_SUFFIX=-d
fi

REL_BIN_DIR=build/bin/$(uname -m)
BINARY_FILE=${REL_BIN_DIR}/${DU_NAME}${BINARY_SUFFIX}

if [ ! -n "$JOBS" ]
then
   JOBS=8
fi

cd $FESA_CLASS_NAME
make -j$JOBS $MAKE_ARG
if [ "$?" != "0" ]
then
   die "Can't build \"$FESA_CLASS_NAME\"!"
fi

cd $ORIGIN_DIR

if [ "$1" = "C" ]
then
   echo -e $ESC_SUCCESS"\"$FESA_CLASS_NAME\" successful build!"$ESC_NORMAL
   exit 0
fi

cd $DU_NAME
make -j$JOBS $MAKE_ARG
if [ "$?" != "0" ]
then
   die "Can't build \"${DU_NAME}\"!"
fi

if [ "$1" = "clean" ]
then
   cd $ORIGIN_DIR
   exit 0
fi

if [ ! -x "$BINARY_FILE" ]
then
   die "Binary \"$BINARY_FILE\" not found!"
fi

if [ ! -d "${EXPORT_DIR}/${DU_NAME}${BINARY_DIR_SUFFIX}" ]
then
   die "Target directory \"${EXPORT_DIR}/${DU_NAME}${BINARY_DIR_SUFFIX}\" not found! Maybe the first build process had not made by Eclipse."
fi

echo -e $ESC_SUCCESS"*** Build was successful, exporting executable: \"${DU_NAME}\" to \"${TARGET}\" ***"$ESC_NORMAL
cp -u ${BINARY_FILE} ${EXPORT_DIR}/${DU_NAME}${BINARY_DIR_SUFFIX}/${DU_NAME}

TEST_DIR=src/test
if [ ! -d "$TEST_DIR" ]
then
   die "Dierctory \"$TEST_DIR\" not found!"
fi

INSTANCE_FILE_DIR=${TEST_DIR}/${TARGET}
if [ ! -d "$INSTANCE_FILE_DIR" ]
then
   die "Dierctory \"$INSTANCE_FILE_DIR\" not found!"
fi

INSTANCE_FILE=${INSTANCE_FILE_DIR}/DeviceData_${DU_NAME}.instance
if [ ! -f "$INSTANCE_FILE" ]
then
   die "Instance file \"$(pwd)/${INSTANCE_FILE}\" not found!"
fi

echo "Exporting \"${DU_NAME}.instance\" to \"${TARGET}\""
cp -u $INSTANCE_FILE ${EXPORT_DIR}/${DU_NAME}${BINARY_DIR_SUFFIX}/${DU_NAME}.instance
if [ "$?" = "0" ]
then
   echo "Content of \"${EXPORT_DIR}/${DU_NAME}${BINARY_DIR_SUFFIX}\":"
   ls -g ${EXPORT_DIR}/${DU_NAME}${BINARY_DIR_SUFFIX}
   echo "done"
fi
cd $ORIGIN_DIR
#=================================== EOF ======================================
