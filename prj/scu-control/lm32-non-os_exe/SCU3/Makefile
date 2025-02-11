###############################################################################
##                                                                           ##
##             Makefile LM32 FG software scu3_control.bin                    ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:    SCU3/Makefile                                                    ##
## (c):     GSI Helmholtz Centre for Heavy Ion Research GmbH                 ##
## Author:  Ulrich Becker                                                    ##
## Date:    12.10.2023                                                       ##
###############################################################################
BASE_VERSION = 4
SUB_VERSION  = 6.10
TARGET  := scu3_control
DEFINES += CONFIG_SCU3
SCU3    := 1 

   MIL_DAQ_USE_RAM := 1

BASE_DIR = $(shell pwd)/..

DEFINES += CONFIG_SCUBUS_INT_RESET_AFTER

# SCU_URL = scuxl0118
### SCU_URL = scuxl0107
# SCU_URL = scuxl0035
# SCU_URL = scuxl0192
# SCU_URL = scuxl0212
# SCU_URL = scuxl0331
# SCU_URL = scuxl0025
# SCU_URL = scuxl0328
# SCU_URL = scuxl0202
# SCU_URL = scuxl0305
# SCU_URL = scuxl0331
#SCU_URL = scuxl0249   # with MIL and ADDAC
# SCU_URL = scuxl0336  # DIOB-test
SCU_URL = scuxl0692
#SCU_URL = scuxl0128



 # ACU-test

include $(BASE_DIR)/scu_control.mk

#=================================== EOF ======================================
