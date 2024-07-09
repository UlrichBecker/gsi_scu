#!/bin/sh
###############################################################################
##                                                                           ##
##                Shell script starts the LM32-log-daemon.                   ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:      start-lm32-logd.sh                                             ##
## Author:    Ulrich Becker                                                  ##
## Date:      09.07.2024                                                     ##
## Copyright: GSI Helmholtz Centre for Heavy Ion Research GmbH               ##
###############################################################################
if [ -f /etc/functions ]
then
   . /etc/functions
else
   alias log='echo INFO: '
   alias log_error='echo ERROR: '
fi

ERROR_LOG=/var/log/error.log
COMMON_INIT_DIR=/opt/nfsinit/global/
LM32_LOG_TARGET=/var/log/lm32.log

# Is it the old RAM-disc?
if [ "$(uname -r)" = "3.10.101-rt111-scu03" ]
then
   # Yes, it is the old RAM-disc "scuxl".
   LM32_LOGD=${COMMON_INIT_DIR}tools/scuxl/lm32-logd
else
   # No, it is the new RAM-disc "yocto".
   LM32_LOGD=${COMMON_INIT_DIR}tools/yocto/lm32-logd
fi

if [ -x "$LM32_LOGD" ]
then
   log "Starting LM32-logging-daemon, target: \"${LM32_LOG_TARGET}\""
   $LM32_LOGD -Habd=${LM32_LOG_TARGET} 2>>$ERROR_LOG
else
   log_error "No LM32-log-daemon present"
fi

if [ -f "$ERROR_LOG" ]
then
   log_error $(cat $ERROR_LOG)
fi

#=================================== EOF ======================================
