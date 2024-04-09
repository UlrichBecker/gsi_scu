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

if [ ! -n "$BINARY_SUFFIX" ]
then
   BINARY_SUFFIX=_S
fi

if [ ! -n "$BINARY_DIR_SUFFIX" ]
then
   BINARY_DIR_SUFFIX=-d
fi

REL_BIN_DIR=build/bin/$(uname -m)
BINARY_FILE=${REL_BIN_DIR}/${DU_NAME}${BINARY_SUFFIX}

cd $FESA_CLASS_NAME
make $1
cd $ORIGIN_DIR

cd $DU_NAME
make $1

if [ "$1" = "clean" ]
then
   cd $ORIGIN_DIR
   exit 0
fi

if [ -x "$BINARY_FILE" ]
then
   if [ -d "$EXPORT_DIR" ]
   then
     echo "Exporting: ${DU_NAME} to ${TARGET}"
     cp -u ${BINARY_FILE} ${EXPORT_DIR}/${DU_NAME}${BINARY_DIR_SUFFIX}/${DU_NAME}
   else
     die "Export directory \"$EXPORT_DIR\" not found!" 1>&2
   fi
else
   die "Binary \"$BINARY_FILE\" not found!" 1>&2
fi
cd $ORIGIN_DIR
#=================================== EOF ======================================
