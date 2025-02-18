#!/bin/sh
###############################################################################
##                                                                           ##
##   Shell script to load and start the LM32 application scu3_control_os     ##
##                        (Vriante with FreeRTOS)                            ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:      start_LM32_scu3_control_os.sh                                  ##
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
LM32_APP=scu3_control_os.bin
source /opt/nfsinit/global/scu_control_release/start_LM32_app.sh

#=================================== EOF ======================================
