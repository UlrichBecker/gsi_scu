#!/bin/sh
###############################################################################
##                                                                           ##
##    Shellscript to start test applications for testing only                ##
##                                                                           ##
##    Make a symbolic link of this script in:                                ##
##    /opt/nfsinit/<scu-name>                                                ##
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

ERROR_LOG=/var/log/error.log
COMMON_INIT_DIR=/opt/nfsinit/global/
LM32_APP_DIR=${COMMON_INIT_DIR}lm32-test-apps/

if [ -n "$(echo $0 | grep 96)" ]
then
   LM32_APP=scu_control_os.bin
else
   LM32_APP=scu_control.bin
fi

EB_FWLOAD=/usr/bin/eb-fwload
EB_RESET=/usr/bin/eb-reset
LM32_LOGD=${COMMON_INIT_DIR}tools/lm32-logd
SOCAT=/usr/bin/socat

$EB_RESET dev/wbm0 cpuhalt 0 2>>$ERROR_LOG
$EB_FWLOAD dev/wbm0 u0 0 ${LM32_APP_DIR}${LM32_APP} 2>>$ERROR_LOG

sleep 3

$LM32_LOGD -Habd=/var/log/lm32.log 2>>$ERROR_LOG

$SOCAT tcp-listen:60368,reuseaddr,fork file:/dev/wbm0 2>>$ERROR_LOG

#=================================== EOF ======================================
