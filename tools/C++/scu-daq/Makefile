###############################################################################
##                                                                           ##
##           Makefile for building the commandline tool "daqt"               ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:     gsi_daq/tools/Makefile                                          ##
## Author:   Ulrich Becker                                                   ##
## Company:  GSI Helmholtz Centre for Heavy Ion Research GmbH                ##
## Date:     11.04.2019                                                      ##
###############################################################################
REPOSITORY_DIR := $(shell git rev-parse --show-toplevel)

DEFINES += VERSION=1.0

MIAN_MODULE := daqt.cpp

SCU_DIR        = $(PRJ_DIR)/scu-control
DAQ_DIR        = $(SCU_DIR)/daq
DAQ_LINUX_DIR  = $(DAQ_DIR)/linux
SDAQ_LINUX_DIR = $(DAQ_LINUX_DIR)/sdaq

DEFINES += CONFIG_AUTODOC_OPTION

SOURCE += daqt_command_line.cpp
SOURCE += daqt_attributes.cpp

SOURCE += $(SCU_LIB_SRC_LINUX_DIR)/daqt_read_stdin.cpp
SOURCE += $(OPT_PARSER_DIR)/parse_opts.cpp
SOURCE += $(GNU_PLOT_STREAM_DIR)/gnuplotstream.cpp
SOURCE += $(LINUX_PROCESS_ID_DIR)/find_process.c
SOURCE += $(SCU_LIB_SRC_LINUX_DIR)/daqt_onFoundProcess.cpp

SOURCE += $(SCU_LIB_SRC_DIR)/fifo/circular_index.c
SOURCE += $(DAQ_LINUX_DIR)/daq_eb_ram_buffer.cpp
SOURCE += $(DAQ_LINUX_DIR)/daq_access.cpp
SOURCE += $(SDAQ_LINUX_DIR)/daq_interface.cpp
SOURCE += $(SDAQ_LINUX_DIR)/daq_administration.cpp
SOURCE += $(DAQ_LINUX_DIR)/daq_base_interface.cpp
SOURCE += $(DAQ_LINUX_DIR)/watchdog_poll.cpp

SOURCE += $(EB_FE_WRAPPER_DIR)/EtherboneConnection.cpp
SOURCE += $(EB_FE_WRAPPER_DIR)/BusException.cpp


GNUPLOT_BIN ?= $(shell which gnuplot)

DEFINES += _BSD_SOURCE
DEFINES += _POSIX_C_SOURCE
DEFINES += _DEFAULT_SOURCE


#DEFINES += DEBUGLEVEL=1
DEFINES += CONFIG_DAQ_TEST
# DEFINES += NDEBUG
DEFINES += CONFIG_DAQ_DEBUG
DEFINES += GPSTR_DEFAULT_GNUPLOT_EXE=\"$(GNUPLOT_BIN)\"

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


HOST ?= $(SCU_URL)

#CALL_ARGS         ?=  -h
#CALL_ARGS         ?= $(HOST) --continue 3 1
#CALL_ARGS         ?= $(HOST) -vS
#CALL_ARGS         ?= $(HOST) 3 -l5 1 3 2 9 3 9 4 -l7 -e
#CALL_ARGS         ?= $(HOST)  -eC 3 1 3 2 3 3 3 4 9 1 9 2 9 3 9 4
#CALL_ARGS         ?= $(HOST) -tEC  3 1
#CALL_ARGS         ?= $(HOST)  -d10 -vtEC 3 1 3 2 -z 9 2
#CALL_ARGS         ?= $(HOST) -Hte 3 1
#CALL_ARGS         ?= $(HOST) -Hrtev 9 1
#CALL_ARGS         ?=  $(HOST) -G /usr/bin/gnuplot -PrzvC 3 1

#CALL_ARGS         ?=  $(HOST) -C=100US 3 1 3 2
#CALL_ARGS         ?=  $(HOST) -C=100US 3 2
#CALL_ARGS         ?=  $(HOST) -Pr 3 2
#CALL_ARGS         ?=  $(HOST) -PrC 3 1 3 2 9 1 9 2
#CALL_ARGS         ?= $(HOST) -n

CALL_ARGS         ?= -N $(HOST) -C 4 2
#CALL_ARGS = $(HOST) -vy=1000
# CALL_ARGS         ?= -vV
REMOTE_CALL_ARGS  ?= "dev/wbm0"


# DEBUG = 1

DOX_INPUT += $(DAQ_DIR)/daq_descriptor.h
DOX_INPUT += $(DAQ_DIR)/daq_command_interface.h
DOX_INPUT += $(DAQ_DIR)/daq_ring_admin.h
DOX_OPTIMIZE_OUTPUT_FOR_C = "NO"


include $(REPOSITORY_DIR)/makefiles/makefile.scun
#=================================== EOF ======================================
