###############################################################################
##                                                                           ##
##     Makefile to create a minimal example of using a feedback channel.     ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:     gsi_daq/example/feedback/Makefile                               ##
## Author:   Ulrich Becker                                                   ##
## Company:  GSI Helmholtz Centre for Heavy Ion Research GmbH                ##
## Date:     26.05.2020                                                      ##
###############################################################################
REPOSITORY_DIR := $(shell git rev-parse --show-toplevel)
DEBUG = 1
# USE_SHARED_LIB := 1
USE_STATIC_LIBS := 1

EB_SRC= $(DAQ_DIR)/feSupport/scu/etherbone

MIAN_MODULE := feedback-example.cpp

SCU_DIR        = $(PRJ_DIR)/scu-control
DAQ_DIR        = $(SCU_DIR)/daq
DAQ_LINUX_DIR  = $(DAQ_DIR)/linux
MDAQ_LINUX_DIR = $(DAQ_LINUX_DIR)/mdaq
SDAQ_LINUX_DIR = $(DAQ_LINUX_DIR)/sdaq


ifndef USE_SHARED_LIB
 DEFINES += DEBUGLEVEL
 SOURCE += $(SCU_LIB_SRC_DIR)/fifo/circular_index.c
 SOURCE += $(DAQ_DIR)/daq_fg_allocator.c
 SOURCE += $(DAQ_LINUX_DIR)/scu_lm32_mailbox.cpp
 SOURCE += $(DAQ_LINUX_DIR)/scu_fg_list.cpp
 SOURCE += $(DAQ_LINUX_DIR)/scu_fg_feedback.cpp
 SOURCE += $(DAQ_LINUX_DIR)/daq_eb_ram_buffer.cpp
 SOURCE += $(DAQ_LINUX_DIR)/daq_access.cpp
 SOURCE += $(DAQ_LINUX_DIR)/daq_base_interface.cpp
 SOURCE += $(DAQ_LINUX_DIR)/watchdog_poll.cpp
 SOURCE += $(MDAQ_LINUX_DIR)/mdaq_interface.cpp
 SOURCE += $(MDAQ_LINUX_DIR)/mdaq_administration.cpp
 SOURCE += $(SDAQ_LINUX_DIR)/daq_administration.cpp
 SOURCE += $(SDAQ_LINUX_DIR)/daq_interface.cpp
 SOURCE += $(SCU_LIB_SRC_LINUX_DIR)/scu_etherbone.cpp
 SOURCE += $(SCU_LIB_SRC_LINUX_DIR)/scu_ddr3_access.cpp
 SOURCE += $(SCU_LIB_SRC_LINUX_DIR)/scu_sram_access.cpp
 SOURCE += $(SCU_LIB_SRC_LINUX_DIR)/scu_lm32_access.cpp
else
 # When a library will used so the following header files
 # has to be told Doxygen explicitly.
 DOX_INPUT += $(SCU_LIB_SRC_DIR)/fifo/circular_index.h
 DOX_INPUT += $(DAQ_LINUX_DIR)/scu_lm32_mailbox.cpp
 DOX_INPUT += $(DAQ_LINUX_DIR)/scu_fg_list.hpp
 DOX_INPUT += $(DAQ_LINUX_DIR)/daq_eb_ram_buffer.hpp
 DOX_INPUT += $(MDAQ_LINUX_DIR)/mdaq_interface.hpp
 DOX_INPUT += $(MDAQ_LINUX_DIR)/mdaq_administration.hpp
endif

DOX_INPUT += $(DAQ_DIR)/daq_ring_admin.h

SOURCE += $(EB_FE_WRAPPER_DIR)/EtherboneConnection.cpp
SOURCE += $(EB_FE_WRAPPER_DIR)/BusException.cpp

INCLUDE_DIRS += $(DAQ_DIR)/tools
INCLUDE_DIRS += $(SCU_LIB_SRC_LM32_DIR)
INCLUDE_DIRS += $(SCU_LIB_SRC_LINUX_DIR)
INCLUDE_DIRS += $(DAQ_LINUX_DIR)
INCLUDE_DIRS += $(SCU_DIR)
INCLUDE_DIRS += $(SCU_DIR)/fg
INCLUDE_DIRS += $(DAQ_DIR)/lm32
INCLUDE_DIRS += $(DAQ_LINUX_DIR)/mdaq
INCLUDE_DIRS += $(DAQ_LINUX_DIR)/sdaq
#INCLUDE_DIRS += $(SCU_DIR)/lm32-non-os_exe/SCU3/generated
INCLUDE_DIRS += $(SCU_DIR)/lm32-rtos_exe/SCU3/generated

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



ifdef USE_SHARED_LIB
 FEEDBACK_LIB = $(DAQ_LINUX_DIR)/feedback/deploy_x86_64/result/libscu_fg_feedback.a
 ADDITIONAL_OBJECTS += $(FEEDBACK_LIB)
endif

LIBS += pthread
LIBS += stdc++


# CALL_ARGS         = "tcp/scuxl0192.acc.gsi.de"
# CALL_ARGS         = "tcp/scuxl0212.acc.gsi.de"
# CALL_ARGS         = "tcp/scuxl0692.acc.gsi.de"
 CALL_ARGS = "tcp/scuxl0249.acc.gsi.de"
REMOTE_CALL_ARGS  ?= "dev/wbm0"

DEBUG = 1

#------------------------------------------------------------------------------
# Additional source code for Doxygen which will not integrated automatically:
DOX_INPUT += $(DAQ_DIR)/generated/shared_mmap.h
DOX_INPUT += $(DAQ_LINUX_DIR)/daq_calculations.hpp
DOX_OPTIMIZE_OUTPUT_FOR_C = "NO"
DOX_RECURSIVE = "YES"
#------------------------------------------------------------------------------

 # NO_LTO = 1

ALL = $(MDAQ_LIB)

include $(REPOSITORY_DIR)/makefiles/makefile.scun

ifdef USE_SHARED_LIB

$(MDAQ_LIB):
	make -C $(MDAQ_LINUX_DIR)
endif
#=================================== EOF ======================================
