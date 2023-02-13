###############################################################################
##                                                                           ##
##           Makefile for building the lm32 lgg daemon lm32-logd             ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:     gsi_daq/tools/lm32-logd/Makefile                                ##
## Author:   Ulrich Becker                                                   ##
## Company:  GSI Helmholtz Centre for Heavy Ion Research GmbH                ##
## Date:     21.04.2022                                                      ##
###############################################################################

DEFINES += VERSION=1.5
FOR_SCU_AND_ACC := 1

DEBUG = 1
#DEFINES += DEBUGLEVEL


MIAN_MODULE := lm32-logd.cpp

SOURCE += logd_cmdline.cpp
SOURCE += logd_core.cpp
SOURCE += $(SCU_LIB_SRC_LINUX_DIR)/daqt_read_stdin.cpp
SOURCE += $(OPT_PARSER_DIR)/parse_opts.cpp
SOURCE += $(LINUX_PROCESS_ID_DIR)/find_process.c
SOURCE += $(SCU_LIB_SRC_DIR)/fifo/circular_index.c
SOURCE += $(SCU_LIB_SRC_DIR)/scu_mmu.c
SOURCE += $(SCU_LIB_SRC_LINUX_DIR)/scu_mmu_fe.cpp
SOURCE += $(SCU_LIB_SRC_LINUX_DIR)/scu_etherbone.cpp
SOURCE += $(SCU_LIB_SRC_LINUX_DIR)/scu_ddr3_access.cpp
SOURCE += $(EB_FE_WRAPPER_DIR)/EtherboneConnection.cpp
SOURCE += $(EB_FE_WRAPPER_DIR)/BusException.cpp

INCLUDE_DIRS += $(PRJ_DIR)/scu-control/daq

DEFINES += CONFIG_AUTODOC_OPTION
DEFINES += CONFIG_SCU_USE_DDR3
DEFINES += _GNU_SOURCE

 USE_STATIC_LIBS := 1

ifdef USE_STATIC_LIBS
 ADDITIONAL_OBJECTS += $(EB_LIB_DIR)/libetherbone.a
else
 LIBS += etherbone
endif

LIBS += stdc++

CALL_ARGS = scuxl0035 -Hbakv -I200 -m1000

DOX_EXTRACT_PRIVATE = YES

REPOSITORY_DIR := $(shell git rev-parse --show-toplevel)
include $(REPOSITORY_DIR)/makefiles/makefile.scun
#=================================== EOF ======================================
