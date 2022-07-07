###############################################################################
##                                                                           ##
##          Makefile for building the commandline tool "mdaqt"               ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:     gsi_daq/tools/mil_daq/Makefile                                  ##
## Author:   Ulrich Becker                                                   ##
## Company:  GSI Helmholtz Centre for Heavy Ion Research GmbH                ##
## Date:     14.08.2019                                                      ##
###############################################################################
REPOSITORY_DIR := $(shell git rev-parse --show-toplevel)
DEFINES += VERSION=0.10

MIAN_MODULE := mdaqt.cpp

SCU_DIR        = $(PRJ_DIR)/scu-control
DAQ_DIR        = $(SCU_DIR)/daq
DAQ_LINUX_DIR  = $(DAQ_DIR)/linux
MDAQ_LINUX_DIR = $(DAQ_LINUX_DIR)/mdaq
SDAQ_LINUX_DIR = $(DAQ_LINUX_DIR)/sdaq

SOURCE += mdaqt_command_line.cpp
SOURCE += mdaq_plot.cpp
SOURCE += $(SCU_LIB_SRC_LINUX_DIR)/daqt_read_stdin.cpp
SOURCE += $(OPT_PARSER_DIR)/parse_opts.cpp
SOURCE += $(GNU_PLOT_STREAM_DIR)/gnuplotstream.cpp
SOURCE += $(LINUX_PROCESS_ID_DIR)/find_process.c
SOURCE += $(SCU_LIB_SRC_LINUX_DIR)/daqt_onFoundProcess.cpp
SOURCE += $(DAQ_LINUX_DIR)/daq_eb_ram_buffer.cpp
SOURCE += $(DAQ_LINUX_DIR)/daq_access.cpp
SOURCE += $(DAQ_LINUX_DIR)/scu_lm32_mailbox.cpp
SOURCE += $(DAQ_LINUX_DIR)/scu_fg_list.cpp
SOURCE += $(MDAQ_LINUX_DIR)/mdaq_interface.cpp
SOURCE += $(MDAQ_LINUX_DIR)/mdaq_administration.cpp
SOURCE += $(DAQ_LINUX_DIR)/daq_base_interface.cpp
SOURCE += $(DAQ_LINUX_DIR)/watchdog_poll.cpp
SOURCE += $(SCU_LIB_SRC_DIR)/fifo/circular_index.c

SOURCE += $(EB_FE_WRAPPER_DIR)/EtherboneConnection.cpp
SOURCE += $(EB_FE_WRAPPER_DIR)/BusException.cpp


GNUPLOT_BIN ?= $(shell which gnuplot)

DEFINES += CONFIG_AUTODOC_OPTION

DEFINES += _BSD_SOURCE
DEFINES += _POSIX_C_SOURCE
# DEFINES += _DEFAULT_SOURCE
# DEFINES += CONFIG_SCU_USE_DDR3
#DEFINES += DEBUGLEVEL=1
DEFINES += CONFIG_DAQ_TEST
# DEFINES += NDEBUG
DEFINES += CONFIG_DAQ_DEBUG
DEFINES += GPSTR_DEFAULT_GNUPLOT_EXE=\"$(GNUPLOT_BIN)\"

# DEFINES += CONFIG_NO_SCU_RAM

# DEFINES += CONFIG_DDR3_NO_BURST_FUNCTIONS

INCLUDE_DIRS += $(SCU_LIB_SRC_LM32_DIR)
INCLUDE_DIRS += $(DAQ_LINUX_DIR)
INCLUDE_DIRS += $(SCU_DIR)
INCLUDE_DIRS += $(SCU_DIR)/fg
INCLUDE_DIRS += $(DAQ_DIR)/lm32
INCLUDE_DIRS += $(DAQ_DIR)
INCLUDE_DIRS += $(DAQ_LINUX_DIR)/mdaq
INCLUDE_DIRS += $(DAQ_LINUX_DIR)/sdaq
INCLUDE_DIRS += $(SCU_DIR)/lm32-non-os_exe/generated

USE_STATIC_LIBS := 1

ifdef USE_STATIC_LIBS
# ADDITIONAL_OBJECTS += $(THIRD_PARTY_DIR)/boost/1.54.0/lib/x86_64/libboost_system.a
 ADDITIONAL_OBJECTS += $(EB_LIB_DIR)/libetherbone.a
else
# LIBS += boost_system
 LIBS += etherbone
endif

LIBS += pthread
LIBS += stdc++


DOX_INPUT += $(DAQ_LINUX_DIR)/daq_calculations.hpp

HOST ?= tcp/$(SCU_URL)


#CALL_ARGS         ?=  $(HOST)

#CALL_ARGS = tcp/scuxl0107 39 130 39 129 39 1 39 2
#CALL_ARGS = -cv tcp/scuxl0107 39 130 39 129
#CALL_ARGS = -c tcp/scuxl0107
CALL_ARGS = tcp/scuxl0162 39 129 -c
#CALL_ARGS = tcp/scuxl0107 -avdc --time 25 -G /home/bel/ubecker/lnx/.local/bin/gnuplot
#CALL_ARGS = scuxl0107 -avcd
#CALL_ARGS = -V
#CALL_ARGS = scuxl0035 -vS
#CALL_ARGS = tcp/scuxl0107 -h
REMOTE_CALL_ARGS  ?= "dev/wbm0"
#CALL_ARGS = tcp/scuxl0107 -v

#CALL_ARGS = --generate_doc_tagged

# DEBUG = 1

DOX_OPTIMIZE_OUTPUT_FOR_C = "NO"

include $(REPOSITORY_DIR)/makefiles/makefile.scun
#=================================== EOF ======================================
