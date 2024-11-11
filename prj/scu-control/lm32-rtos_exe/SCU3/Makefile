###############################################################################
##                                                                           ##
##       Makefile LM32 FG software scu3_control_os.bin using FreeRTOS        ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:    SCU3/Makefile                                                    ##
## (c):     GSI Helmholtz Centre for Heavy Ion Research GmbH                 ##
## Author:  Ulrich Becker                                                    ##
## Date:    12.10.2023                                                       ##
###############################################################################
BASE_VERSION = 4
SUB_VERSION  = 6.9

DEFINES += CONFIG_HANDLE_UNUSED_ECAS

#------------------------------------------------------------------------------
 SCU_MIL := 1
# MIL_GAP := 1

 USE_ADDAC_FG_TASK := 1

 USE_TEMPERATURE_WATCHER :=1

 MIL_DAQ_USE_RAM := 1
 ADDAC_DAQ := 1
 DIOB_WITH_DAQ := 1
# DEFINES += _CONFIG_DBG_MIL_TASK

#NEW_ADDAC_HANDSHAKE := 1

TARGET  := scu3_control_os
DEFINES += CONFIG_SCU3
SCU3    := 1

BASE_DIR = $(shell pwd)/..
# DEFINES += DEBUGLEVEL
#DEFINES +=  CONFIG_IRQ_RESET_IP_AFTER
STACK_SIZE  =  512
#NO_LTO := 1

# SCU_URL = scuxl0118
# SCU_URL = scuxl0192
# SCU_URL = scuxl0212
# SCU_URL = scuxl0331
# SCU_URL = scuxl0025
# SCU_URL = scuxl0328
# SCU_URL = scuxl0202
# SCU_URL = scuxl0305
# SCU_URL = scuxl0331
# SCU_URL = scuxl0162   # with MIL and ADDAC
# SCU_URL = scuxl0336  # DIOB-test
SCU_URL = scuxl0692
 # ACU-test

include $(BASE_DIR)/scu_control_os.mk
#=================================== EOF ======================================
