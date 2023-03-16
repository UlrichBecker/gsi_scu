###############################################################################
##                                                                           ##
##        Makefile for building the tool "memory-monitor" ("mem-mom")        ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:     gsi_daq/tools/mem-mon/Makefile                                  ##
## Author:   Ulrich Becker                                                   ##
## Company:  GSI Helmholtz Centre for Heavy Ion Research GmbH                ##
## Date:     12.04.2022                                                      ##
###############################################################################

DEFINES += VERSION=2.1
FOR_SCU_AND_ACC := 1

USE_STATIC_LIBS := 1
USE_NAMED_MUTEX := 1

MIAN_MODULE := mem-mon.cpp

SOURCE += mem_browser.cpp
SOURCE += mem_cmdline.cpp
SOURCE += $(OPT_PARSER_DIR)/parse_opts.cpp
SOURCE += $(SCU_LIB_SRC_DIR)/scu_mmu.c
SOURCE += $(SCU_LIB_SRC_LINUX_DIR)/scu_mmu_fe.cpp
SOURCE += $(SCU_LIB_SRC_LINUX_DIR)/scu_etherbone.cpp
SOURCE += $(SCU_LIB_SRC_LINUX_DIR)/scu_ddr3_access.cpp
SOURCE += $(SCU_LIB_SRC_LINUX_DIR)/scu_sram_access.cpp
SOURCE += $(EB_FE_WRAPPER_DIR)/EtherboneConnection.cpp
SOURCE += $(EB_FE_WRAPPER_DIR)/BusException.cpp

DEFINES += CONFIG_AUTODOC_OPTION
DEFINES += CONFIG_SCU_USE_DDR3

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

# DEBUG = 1
# DEFINES += CONFIG_DEBUG_MESSAGES

CALL_ARGS = scuxl0035 -va1,10 -a2,200 -a0x4711,3001 -a0xabcd,1000000
#CALL_ARGS = scuxl0035 -vD

REPOSITORY_DIR := $(shell git rev-parse --show-toplevel)
include $(REPOSITORY_DIR)/makefiles/makefile.scun
#=================================== EOF ======================================
