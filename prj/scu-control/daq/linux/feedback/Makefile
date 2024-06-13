###############################################################################
##                                                                           ##
##  Makefile to generate the static library libscu_fg_feedback.a for MIL-    ##
##       and ADDAC/ACU- DAQs for function generator feedback.                ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:    gsi_daq/linux/feedback/Makefile                                  ##
## Author:  Ulrich Becker <u.becker@gsi.de>                                  ##
## Company: GSI Helmholtz Centre for Heavy Ion Research GmbH                 ##
## Date:    14.10.2020                                                       ##
###############################################################################
STATIC_LIBRARY := 1
LIBRRARY_NAME := scu_fg_feedback

#DEBUG = 1

ifdef DEBUG
  TARGET := $(LIBRRARY_NAME)_dbg
  DEFINES += CONFIG_DEBUG_MESSAGES
else
  TARGET := $(LIBRRARY_NAME)
endif

ifndef OECORE_SDK_VERSION
ifneq ($(shell echo $${HOSTNAME:0:5}), asl74)
   $(error ERROR: This library has to be build by Yocto-SDK or on ACC7)
endif
endif

$(info Building static library $(TARGET) )

SCU_DIR        = $(PRJ_DIR)/scu-control
DAQ_DIR        = $(SCU_DIR)/daq
DAQ_LINUX_DIR  = $(DAQ_DIR)/linux
MDAQ_LINUX_DIR = $(DAQ_LINUX_DIR)/mdaq
SDAQ_LINUX_DIR = $(DAQ_LINUX_DIR)/sdaq

GENERATED_LM32_DIR = $(SCU_DIR)/lm32-non-os_exe/SCU3/generated

INCLUDE_DIRS += $(SCU_LIB_SRC_LINUX_DIR)
# INCLUDE_DIRS += $(EB_FE_WRAPPER_DIR)
INCLUDE_DIRS += $(ETHERBONE_DIR)/etherbone-core/api
INCLUDE_DIRS += $(DAQ_LINUX_DIR)
INCLUDE_DIRS += $(DAQ_DIR)
INCLUDE_DIRS += $(DAQ_DIR)/lm32
INCLUDE_DIRS += $(SCU_DIR)
INCLUDE_DIRS += $(SCU_LIB_SRC_LM32_DIR)
INCLUDE_DIRS += $(SCU_DIR)/fg
INCLUDE_DIRS += $(GENERATED_LM32_DIR)

SOURCE =  $(MDAQ_LINUX_DIR)/mdaq_administration.cpp
SOURCE += $(MDAQ_LINUX_DIR)/mdaq_interface.cpp
SOURCE += $(SDAQ_LINUX_DIR)/daq_interface.cpp
SOURCE += $(SDAQ_LINUX_DIR)/daq_administration.cpp
SOURCE += $(DAQ_LINUX_DIR)/daq_eb_ram_buffer.cpp
SOURCE += $(DAQ_LINUX_DIR)/daq_access.cpp
SOURCE += $(DAQ_LINUX_DIR)/scu_lm32_mailbox.cpp
SOURCE += $(DAQ_LINUX_DIR)/scu_fg_list.cpp
SOURCE += $(DAQ_LINUX_DIR)/watchdog_poll.cpp
SOURCE += $(DAQ_LINUX_DIR)/daq_base_interface.cpp
SOURCE += $(DAQ_LINUX_DIR)/scu_fg_feedback.cpp
SOURCE += $(SCU_LIB_SRC_DIR)/fifo/circular_index.c
SOURCE += $(SCU_LIB_SRC_LINUX_DIR)/scu_etherbone.cpp
SOURCE += $(SCU_LIB_SRC_LINUX_DIR)/scu_ddr3_access.cpp
SOURCE += $(SCU_LIB_SRC_LINUX_DIR)/scu_sram_access.cpp
SOURCE += $(SCU_LIB_SRC_LINUX_DIR)/scu_lm32_access.cpp
SOURCE += $(DAQ_DIR)/daq_fg_allocator.c

# NO_LTO = 1

ifeq ($(shell echo $${HOSTNAME:0:4}), asl7)
   DOX_OUTPUT_DIRECTORY = /common/usr/cscofe/doc/scu/daq/$(TARGET)
endif

DOX_EXPAND_AS_DEFINED      = ADD_NAMESPACE

DOX_EXAMPLE_PATH = $(REPOSITORY_DIR)/demo-and-test/linux/feedback/

DOX_EXTRACT_PRIVATE = "YES"

DOX_INPUT += $(DAQ_LINUX_DIR)/feedback/scu_feedback.md
DOX_INPUT += $(DOX_EXAMPLE_PATH)/feedback-example.cpp
DOX_FILE_PATTERNS += *.md

REPOSITORY_DIR := $(shell git rev-parse --show-toplevel)
include $(REPOSITORY_DIR)/makefiles/makefile.scun
#=================================== EOF ======================================
