###############################################################################
##                                                                           ##
##           Makefile for building the test-application ddr3iot              ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:    gsi_scu/demo-and-test/ddr3/linux/Makefile                        ##
## Author:  Ulrich Becker                                                    ##
## Company: GSI Helmholtz Centre for Heavy Ion Research GmbH                 ##
## Date:    30.01.2023                                                       ##
###############################################################################

FOR_SCU_AND_ACC := 1
DEBUG = 1
USE_STATIC_LIBS := 1
USE_NAMED_MUTEX := 1
MIAN_MODULE := ddr3iot.cpp

SOURCE += $(EB_FE_WRAPPER_DIR)/EtherboneConnection.cpp
SOURCE += $(EB_FE_WRAPPER_DIR)/BusException.cpp
SOURCE += $(SCU_LIB_SRC_LINUX_DIR)/scu_etherbone.cpp
SOURCE += $(SCU_LIB_SRC_LINUX_DIR)/scu_ddr3_access.cpp


ifdef USE_NAMED_MUTEX
 LIBS += pthread
else
 DEFINES += CONFIG_EB_USE_NORMAL_MUTEX
endif
ifdef USE_STATIC_LIBS
 ifdef USE_NAMED_MUTEX
  ADDITIONAL_OBJECTS += $(BOOST_LIB_DIR)/libboost_system.a
 endif
  ADDITIONAL_OBJECTS += $(EB_LIB_DIR)/libetherbone.a
else
 ifdef USE_NAMED_MUTEX
  LIBS += boost_system
 endif
 LIBS += etherbone
endif

LIBS += stdc++


INCLUDE_DIRS += $(SCU_LIB_SRC_LINUX_DIR)

CALL_ARGS = scuxl0035

REPOSITORY_DIR := $(shell git rev-parse --show-toplevel)
include $(REPOSITORY_DIR)/makefiles/makefile.scun
#=================================== EOF ======================================
