###############################################################################
##                                                                           ##
##          Makefile for building the commandline tool "fg-wave"             ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:     gsi_daq/tools/fg-wave/Makefile                                  ##
## Author:   Ulrich Becker                                                   ##
## Company:  GSI Helmholtz Centre for Heavy Ion Research GmbH                ##
## Date:     07.12.2020                                                      ##
###############################################################################
DEFINES += VERSION=1.4

MIAN_MODULE := fg-wave.cpp

SCU_DIR        = $(PRJ_DIR)/scu-control
DAQ_DIR        = $(SCU_DIR)/daq
DAQ_LINUX_DIR  = $(DAQ_DIR)/linux
SDAQ_LINUX_DIR = $(DAQ_LINUX_DIR)/sdaq



SOURCE += fgw_commandline.cpp
SOURCE += fgw_polynom.cpp
SOURCE += $(OPT_PARSER_DIR)/parse_opts.cpp
SOURCE += $(GNU_PLOT_STREAM_DIR)/gnuplotstream.cpp
SOURCE += $(SCU_LIB_SRC_LINUX_DIR)/fgw_parser.cpp
SOURCE += $(SCU_LIB_SRC_LINUX_DIR)/daqt_read_stdin.cpp

GNUPLOT_BIN ?= $(shell which gnuplot)

DEFINES += CONFIG_AUTODOC_OPTION
DEFINES += _SCU_CONTROL_CONFIG_H
DEFINES += _DEFAULT_SOURCE
#DEFINES += _BSD_SOURCE
#DEFINES += _POSIX_C_SOURCE

# DEFINES += NDEBUG

# DEFINES += DEBUGLEVEL
DEFINES += GPSTR_DEFAULT_GNUPLOT_EXE=\"$(GNUPLOT_BIN)\"

# DEFINES += CONFIG_NO_SCU_RAM

# DEFINES += CONFIG_DDR3_NO_BURST_FUNCTIONS

INCLUDE_DIRS += $(DAQ_LINUX_DIR)
INCLUDE_DIRS += $(DAQ_DIR)
INCLUDE_DIRS += $(SCU_DIR)/fg

# LIBS += pthread
LIBS += stdc++
# LIBS += m

# CALL_ARGS = ../../../gsi_scu/sinus-test.txt
#CALL_ARGS = sinus-test500.fgw -c
CALL_ARGS = ramp_ifa_scu.fgw -c
#CALL_ARGS = gs11mu2-volt-raw-all.fgw -c
# CALL_ARGS = -r3 sinus.fgw
# CALL_ARGS = -z gs11mu2-curr-raw-p1.fgw 
#CALL_ARGS = -zd100 q.fgw
#CALL_ARGS = -zd100 q.fgw
#  DEBUG = 1


DOX_OPTIMIZE_OUTPUT_FOR_C = "NO"

REPOSITORY_DIR := $(shell git rev-parse --show-toplevel)
include $(REPOSITORY_DIR)/makefiles/makefile.scun
#=================================== EOF ======================================
