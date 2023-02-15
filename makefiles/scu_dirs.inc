###############################################################################
##                                                                           ##
##    Defining of some directory variables and include directories of SCU    ##
##                          for SCU makefiles                                ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:    scu_dirs.inc                                                     ##
## (c):     GSI Helmholtz Centre for Heavy Ion Research GmbH                 ##
## Author:  Ulrich Becker                                                    ##
## Date:    22.01.2019                                                       ##
###############################################################################

PRJ_DIR = $(REPOSITORY_DIR)/prj

SCU_LIB_SRC_DIR       = $(REPOSITORY_DIR)/srclib
SCU_LIB_SRC_LM32_DIR  = $(SCU_LIB_SRC_DIR)/lm32
SCU_LIB_SRC_LINUX_DIR = $(SCU_LIB_SRC_DIR)/linux

IP_CORE_DIR     = $(SCU_LIB_SRC_DIR)/timing/wr
SAFTLIB_DIR     = $(SCU_LIB_SRC_DIR)/timing/saftlib
WR_DIR          = $(IP_CORE_DIR)
WR_TOOLS        = $(SCU_LIB_SRC_DIR)/timing/tools

THIRD_PARTY_DIR = /opt/gsi/3rdparty

BOOST_VERSION   ?= 1.54.0
BOOST_DIR       ?= $(THIRD_PARTY_DIR)/boost/$(BOOST_VERSION)
BOOST_LIB_DIR    = $(BOOST_DIR)/lib/x86_64

INCLUDE_DIRS += $(SCU_LIB_SRC_DIR)
INCLUDE_DIRS += $(WR_DIR)
INCLUDE_DIRS += $(SAFTLIB_DIR)
INCLUDE_DIRS += $(SCU_LIB_SRC_DIR)/fifo

SCU_VERSION ?= 3
DEFINES += CONFIG_SCU=$(SCU_VERSION)

# ------------ Additional defines for Doxygen ---------------------------------
DOX_OUTPUT_DIRECTORY ?= $(HOME)/Documents/scr_doc/$(TARGET)
DOX_PROJECT_LOGO     ?= $(MAKEFILE_DIR)/fair_gsi_logo63x55.png
DOX_INPUT            += $(MAKEFILE_DIR)/scu_dirs.inc
# -------------- End of defines for Doxygen -----------------------------------
#=================================== EOF ======================================
