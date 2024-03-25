###############################################################################
##                                                                           ##
##        Makefile for building the commandline tool "fg-feedback"           ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:     gsi_daq/tools/fg-feedback/Makefile                              ##
## Author:   Ulrich Becker                                                   ##
## Company:  GSI Helmholtz Centre for Heavy Ion Research GmbH                ##
## Date:     09.10.2020                                                      ##
###############################################################################
REPOSITORY_DIR := $(shell git rev-parse --show-toplevel)
DEFINES += VERSION=4.6.2
FOR_SCU_AND_ACC := 1

#USE_NAMED_MUTEX := 1

USE_ADDAC_DAQ_BLOCK_STATISTICS := 1
USE_TUPLE_STATISTICS := 1

ifdef USE_ADDAC_DAQ_BLOCK_STATISTICS
   DEFINES += CONFIG_USE_ADDAC_DAQ_BLOCK_STATISTICS
endif
ifdef USE_TUPLE_STATISTICS
   DEFINES += CONFIG_USE_TUPLE_STATISTICS
endif

MIAN_MODULE := fg-feedback.cpp

SCU_DIR        = $(PRJ_DIR)/scu-control
DAQ_DIR        = $(SCU_DIR)/daq
DAQ_LINUX_DIR  = $(DAQ_DIR)/linux
MDAQ_LINUX_DIR = $(DAQ_LINUX_DIR)/mdaq
SDAQ_LINUX_DIR = $(DAQ_LINUX_DIR)/sdaq

SOURCE += fb_command_line.cpp
SOURCE += fb_plot.cpp
ifdef USE_ADDAC_DAQ_BLOCK_STATISTICS
  SOURCE += daq_statistics.cpp
endif
ifdef USE_TUPLE_STATISTICS
  SOURCE += tuple_statistics.cpp
endif

SOURCE += $(SCU_LIB_SRC_LINUX_DIR)/daqt_read_stdin.cpp
SOURCE += $(OPT_PARSER_DIR)/parse_opts.cpp
SOURCE += $(GNU_PLOT_STREAM_DIR)/gnuplotstream.cpp
SOURCE += $(LINUX_PROCESS_ID_DIR)/find_process.c
SOURCE += $(SCU_LIB_SRC_LINUX_DIR)/daqt_onFoundProcess.cpp
SOURCE += $(SCU_LIB_SRC_LINUX_DIR)/scu_etherbone.cpp
SOURCE += $(SCU_LIB_SRC_LINUX_DIR)/scu_ddr3_access.cpp
SOURCE += $(SCU_LIB_SRC_LINUX_DIR)/scu_sram_access.cpp
SOURCE += $(SCU_LIB_SRC_LINUX_DIR)/scu_lm32_access.cpp


SOURCE += $(SCU_LIB_SRC_DIR)/fifo/circular_index.c
SOURCE += $(DAQ_DIR)/daq_fg_allocator.c

SOURCE += $(DAQ_LINUX_DIR)/scu_lm32_mailbox.cpp
SOURCE += $(DAQ_LINUX_DIR)/scu_fg_list.cpp
SOURCE += $(DAQ_LINUX_DIR)/scu_fg_feedback.cpp
SOURCE += $(DAQ_LINUX_DIR)/daq_eb_ram_buffer.cpp
SOURCE += $(DAQ_LINUX_DIR)/daq_access.cpp
SOURCE += $(DAQ_LINUX_DIR)/watchdog_poll.cpp
SOURCE += $(DAQ_LINUX_DIR)/daq_base_interface.cpp
SOURCE += $(MDAQ_LINUX_DIR)/mdaq_interface.cpp
SOURCE += $(MDAQ_LINUX_DIR)/mdaq_administration.cpp
SOURCE += $(SDAQ_LINUX_DIR)/daq_administration.cpp
SOURCE += $(SDAQ_LINUX_DIR)/daq_interface.cpp

SOURCE += $(EB_FE_WRAPPER_DIR)/EtherboneConnection.cpp
SOURCE += $(EB_FE_WRAPPER_DIR)/BusException.cpp


GNUPLOT_BIN ?= $(shell which gnuplot)

DEFINES += CONFIG_AUTODOC_OPTION

ifndef USE_NAMED_MUTEX
 DEFINES += CONFIG_EB_USE_NORMAL_MUTEX
endif

#DEBUG = 1
#DEFINES += CONFIG_DEBUG_MESSAGES
# DEFINES += CONFIG_DAQ_TEST
# DEFINES += NDEBUG
# DEFINES += CONFIG_DAQ_DEBUG
# DEFINES += DEBUGLEVEL
DEFINES += GPSTR_DEFAULT_GNUPLOT_EXE=\"$(GNUPLOT_BIN)\"

# DEFINES += CONFIG_NO_SCU_RAM

# DEFINES += CONFIG_DDR3_NO_BURST_FUNCTIONS

INCLUDE_DIRS += $(SCU_LIB_SRC_LM32_DIR)
INCLUDE_DIRS += $(DAQ_LINUX_DIR)
INCLUDE_DIRS += $(SCU_DIR)
INCLUDE_DIRS += $(SCU_DIR)/fg
INCLUDE_DIRS += $(DAQ_DIR)/lm32
INCLUDE_DIRS += $(DAQ_LINUX_DIR)/mdaq
INCLUDE_DIRS += $(DAQ_LINUX_DIR)/sdaq
INCLUDE_DIRS += $(SCU_DIR)/lm32-non-os_exe/SCU3/generated

 USE_STATIC_LIBS := 1

ifdef USE_STATIC_LIBS
 ifdef USE_NAMED_MUTEX
  ADDITIONAL_OBJECTS += $(THIRD_PARTY_DIR)/boost/1.54.0/lib/x86_64/libboost_system.a
 endif
  ADDITIONAL_OBJECTS += $(EB_LIB_DIR)/libetherbone.a
else
 ifdef USE_NAMED_MUTEX
  LIBS += boost_system
 endif
 LIBS += etherbone
endif

LIBS += pthread
LIBS += stdc++

NO_LTO := 1

DOX_INPUT += $(DAQ_LINUX_DIR)/daq_calculations.hpp

HOST ?= tcp/$(SCU_URL)


#CALL_ARGS         ?=  $(HOST)

#CALL_ARGS = tcp/scuxl0107 -v 39 130 39 129 39 1
#CALL_ARGS = -cv tcp/scuxl0107 39 130 39 129
#CALL_ARGS = -c tcp/scuxl0107
#CALL_ARGS = tcp/scuxl0107 39 130 -c
#CALL_ARGS = tcp/scuxl0107 -avdc --time 25 -G /home/bel/ubecker/lnx/.local/bin/gnuplot
#CALL_ARGS = scuxl0107 -avcd
#CALL_ARGS = -V
#CALL_ARGS = scuxl0118 -vS
#CALL_ARGS = tcp/scuxl0107 -h
REMOTE_CALL_ARGS  ?= "dev/wbm0"
#CALL_ARGS = tcp/scuxl0107 -vS

#CALL_ARGS = --generate_doc_tagged

# CALL_ARGS = scuxl0692.acc.gsi.de -vadt1
# CALL_ARGS = scuxl0692.acc.gsi.de -vS
 CALL_ARGS = scuxl0692.acc.gsi.de -aN
#CALL_ARGS = scuxl0212 -aqzt1 -I200 -T"X11 size 800,150"
#CALL_ARGS = scuxl0692.acc.gsi.de -azrdQOt1 -I250 -T"X11 size 800,150"  -u0
#CALL_ARGS = scuxl0692.acc.gsi.de -I200 -rQ
#CALL_ARGS = scuxl0162 -azt1 -I200 -T"X11 size 800,150"

# CALL_ARGS = scuxl0035 -vy

# CALL_ARGS = scuxl0035 -b11,12
# CALL_ARGS = scuxl0035 -azdt1 -I10000 -T"X11 size 800,150
# CALL_ARGS = "140.181.160.53" -azt1 -T"X11 size 800,150"
# CALL_ARGS = scuxl0107 -azdt1 -T"X11 size 800,150"


DOX_OPTIMIZE_OUTPUT_FOR_C = "NO"

include $(REPOSITORY_DIR)/makefiles/makefile.scun
#=================================== EOF ======================================
