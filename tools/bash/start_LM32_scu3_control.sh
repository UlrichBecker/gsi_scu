#!/bin/sh
###############################################################################
##                                                                           ##
##   Shell script to load and start the LM32 application scu3_control_os     ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:      start_LM32_scu3_control.sh                                     ##
## Author:    Ulrich Becker                                                  ##
## Date:      10.02.2025                                                     ##
## Copyright: GSI Helmholtz Centre for Heavy Ion Research GmbH               ##
###############################################################################
#
# Location of this script on ASL: /common/export/nfsinit/global/scu_control_release/
# Location of this script on SCU:           /opt/nfsinit/global/scu_control_release/
#
# Make a symbolic link of this script in: /opt/nfsinit/<scu-name>:
#

if [ -f /etc/functions ]
then
   . /etc/functions
else
   alias log='echo INFO: '
   alias log_error='echo ERROR: '
fi

ERROR_LOG=/var/log/error.log
COMMON_INIT_DIR=/opt/nfsinit/global/
LM32_APP_DIR=${COMMON_INIT_DIR}scu_control_release/

if [ -f "$ERROR_LOG" ]
then
   rm $ERROR_LOG
fi

LM32_APP=scu3_control_os.bin   # Version with FreeRTOS
#LM32_APP=scu3_control.bin      # Version without FreeRTOS

EB_FWLOAD=/usr/bin/eb-fwload
EB_RESET=/usr/bin/eb-reset
SAFT_SCU_CTL=/usr/bin/saft-scu-ctl

LM32_LOG_TARGET=/var/log/lm32.log


if [ -f "${LM32_APP_DIR}${LM32_APP}" ]
then
   if [ -x "$EB_RESET" ]
   then
      log "Reseting LM32"
      $EB_RESET dev/wbm0 cpuhalt 0 2>>$ERROR_LOG
   else
      log_error "\"eb-reset\" not found!"
   fi
   if [ -x "$EB_FWLOAD" ]
   then
      if [ -x "$SAFT_SCU_CTL" ]
      then
         log "Creating ECA-condition for synchronizing the DAQ timestamp-counter."
         $SAFT_SCU_CTL tr0 -d -c 0xDADADADABABABABA 0xFFFFFFFFFFFFFFFF 0 0xCAFEAFFE
      else
         log_error "\"saft-scu-ctl\" not found!"
      fi
      log "Upload and starting LM32-application: \"$LM32_APP\""
      $EB_FWLOAD dev/wbm0 u0 0 ${LM32_APP_DIR}${LM32_APP} 2>>$ERROR_LOG
      sleep 3
   else
      log_error "\"eb-fwload\" not found!"
   fi
else
   log_error "LM32-application: \"${LM32_APP_DIR}${LM32_APP}\" not found!"
fi

if [ -f "$ERROR_LOG" ] && [ -n "$(cat $ERROR_LOG)" ]
then
   log_error $(cat $ERROR_LOG)
fi

#=================================== EOF ======================================
