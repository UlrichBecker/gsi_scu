#!/bin/sh
###############################################################################
##                                                                           ##
##               Shell script to start test applications.                    ##
##          CAUTION: For testing and development purposes only!              ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:      scu_test_init.sh                                               ##
## Author:    Ulrich Becker                                                  ##
## Date:      02.09.2022                                                     ##
## Copyright: GSI Helmholtz Centre for Heavy Ion Research GmbH               ##
###############################################################################
#
# Location of this script on ASL: /common/export/nfsinit/global/lm32-test-apps/
# Location of this script on SCU:           /opt/nfsinit/global/lm32-test-apps/
#
# Make a symbolic link of this script in: /opt/nfsinit/<scu-name>:
#
# * Symbolic link name for testing the FreeRTOS version "scu3_control_os.bin"
#   96_scu_test_init
#
# * Symbolic link name for testing the classical version "scu3_control.bin"
#   95_scu_test_init
#
# You can easily switch between both versions by renaming of the link-name.
#

ERROR_LOG=/var/log/error.log
COMMON_INIT_DIR=/opt/nfsinit/global/
LM32_APP_DIR=${COMMON_INIT_DIR}lm32-test-apps/

if [ -n "$(echo $0 | grep 96)" ]
then
   LM32_APP=scu3_control_os.bin   # Version with FreeRTOS (beta!)
else
   LM32_APP=scu3_control.bin      # Version without FreeRTOS
fi

. /etc/functions

EB_FWLOAD=/usr/bin/eb-fwload
EB_RESET=/usr/bin/eb-reset

if [ "$(uname -r)" = "3.10.101-rt111-scu03" ]
then
   LM32_LOGD=${COMMON_INIT_DIR}tools/scuxl/lm32-logd
else
   LM32_LOGD=${COMMON_INIT_DIR}tools/yocto/lm32-logd
fi

LM32_LOG_TARGET=/var/log/lm32.log

SOCAT=/usr/bin/socat

if [ -f "${LM32_APP_DIR}${LM32_APP}" ]
then
   log "Reseting LM32"
   $EB_RESET dev/wbm0 cpuhalt 0 2>>$ERROR_LOG
   log "Upload and starting LM32-application: \"$LM32_APP\""
   $EB_FWLOAD dev/wbm0 u0 0 ${LM32_APP_DIR}${LM32_APP} 2>>$ERROR_LOG
   sleep 3
else
   log_error "LM32-application: \"${LM32_APP_DIR}${LM32_APP}\" not found!"
fi

if [ ! -x "$LM32_LOGD" ]
then
   log_error "No LM32-log-daemon present"
else
   log "Starting LM32-logging-daemon, target: \"${LM32_LOG_TARGET}\""
   $LM32_LOGD -Habd=${LM32_LOG_TARGET} 2>>$ERROR_LOG
fi

log "Starting \"socat\""
$SOCAT tcp-listen:60368,reuseaddr,fork file:/dev/wbm0 2>>$ERROR_LOG

if [ -f "$ERROR_LOG" ]
then
   log_error $(cat $ERROR_LOG)
fi

#=================================== EOF ======================================
