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
# Location pdf this script: /common/export/nfsinit/global/
#

LM32_APP=scu_control.bin
#LM32_APP=scu_control_OS.bin
LM32_APP_DIR="../global/lm32-test-apps/"

EB_FWLOAD=/usr/bin/eb-fwload
EB_RESET=/usr/bin/eb-reset
LM32_LOGD=/opt/nfsinit/global/tools/lm32-logd
SOCAT=/usr/bin/socat


$EB_RESET dev/wbm0 cpuhalt 0
$EB_FWLOAD dev/wbm0 u0 0 ${LM32_APP_DIR}/${LM32_APP}

sleep 3

$LM32_LOGD -Habd=/var/log/lm32.log

$SOCAT tcp-listen:60368,reuseaddr,fork file:/dev/wbm0

#=================================== EOF ======================================
