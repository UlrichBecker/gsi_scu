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
SUB_VERSION  = 6.3

#------------------------------------------------------------------------------
 SCU_MIL := 1
# MIL_GAP := 1

 USE_TEMPERATURE_WATCHER :=1

 MIL_DAQ_USE_RAM := 1
 ADDAC_DAQ := 1
 DIOB_WITH_DAQ := 1
# DEFINES += _CONFIG_DBG_MIL_TASK

NEW_ADDAC_HANDSHAKE := 1

TARGET  := scu3_control_os
DEFINES += CONFIG_SCU3
SCU3    := 1

BASE_DIR = $(shell pwd)/..
# DEFINES += DEBUGLEVEL

STACK_SIZE  =  512
#NO_LTO := 1
include $(BASE_DIR)/scu_control_os.mk
#=================================== EOF ======================================
