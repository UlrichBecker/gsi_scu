###############################################################################
##                                                                           ##
##              Makefile LM32 FG software scu_control.bin                    ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:    Makefile                                                         ##
## (c):     GSI Helmholtz Centre for Heavy Ion Research GmbH                 ##
## Author:  Ulrich Becker                                                    ##
## Date:    09.01.2019                                                       ##
###############################################################################
BASE_VERSION = 4
SUB_VERSION  = 5.2
TARGET       := scu_control
#------------------------------------------------------------------------------
# USE_HISTORY=1
 USE_LM32LOG=1
 USE_MMU := 1
 SCU_MIL := 1
# MIL_GAP := 1

MIL_DAQ_USE_DDR3 := 1

ifdef SCU_MIL
 MIL_IN_TIMER_INTERRUPT := 1
endif
 ADDAC_DAQ := 1
 DIOB_WITH_DAQ := 1
# DEFINES += _CONFIG_DBG_MIL_TASK

NEW_ADDAC_HANDSHAKE := 1

#------------------------------------------------------------------------------

ifdef USE_HISTORY
 ifdef USE_LM32LOG
  $(error Either History or LM32-logsystem but not both! )
 endif
endif
ifdef USE_LM32LOG
  USE_MMU := 1
endif

ifdef NEW_ADDAC_HANDSHAKE
 DEFINES += _CONFIG_WAS_READ_FOR_ADDAC_DAQ
endif

# DEFINES += CONFIG_DBG_MEASURE_IRQ_TIME
# DEFINES += CONFIG_FG_PEDANTIC_CHECK
# DEFINES += CONFIG_DAQ_PEDANTIC_CHECK
 
# Avoiding of including of scu_control_config.h
DEFINES += _SCU_CONTROL_CONFIG_H

 DEFINES += _CONFIG_IRQ_ENABLE_IN_START_FG
 DEFINES += _CONFIG_ECA_BY_MSI
# DEFINES += CONFIG_LOG_ALL_SIGNALS

# DEFINES += _CONFIG_NEW

DEFINES += FG_VERSION=$(BASE_VERSION)
VERSION_STR = $(BASE_VERSION).$(SUB_VERSION)
DEFINES += SW_VERSION=$(BASE_VERSION).$(SUB_VERSION)

ifdef NEW_ADDAC_HANDSHAKE
 VERSION_STR += "*"
endif

ifeq ($(BASE_VERSION), 3)
  DEFINES += CONFIG_FW_VERSION_3
endif

DEFINES += CONFIG_USE_FG_MSI_TIMEOUT

ifdef MIL_IN_TIMER_INTERRUPT
 DEFINES += MAX_LM32_INTERRUPTS=2
 DEFINES += CONFIG_MIL_IN_TIMER_INTERRUPT
 VERSION_STR += "+TI"
else
 DEFINES += MAX_LM32_INTERRUPTS=1
endif
 DEFINES += CONFIG_USE_INTERRUPT_TIMESTAMP
# DEFINES += CONFIG_IRQ_RESET_IP_AFTER
# DEFINES += CONFIG_DISABLE_CRITICAL_SECTION
 DEFINES += CONFIG_INTERRUPT_PEDANTIC_CHECK
 DEFINES +=  CONFIG_IRQ_ENABLING_IN_ATOMIC_SECTIONS

 DEFINES += MICO32_FULL_CONTEXT_SAVE_RESTORE



# DEFINES += DEBUGLEVEL
MIAN_MODULE    = scu_main.c


include ../common_make.mk

SOURCE += $(SCU_LIB_SRC_LM32_DIR)/sys/lm32Interrupts.c

# NO_LTO=1
 STACK_SIZE  =  4096

# SCU_URL = scuxl0118
### SCU_URL = scuxl0107
# SCU_URL = scuxl0035
# SCU_URL = scuxl0192
# SCU_URL = scuxl0212
# SCU_URL = scuxl0331
# SCU_URL = scuxl0025
# SCU_URL = scuxl0328
# SCU_URL = scuxl0202
# SCU_URL = scuxl0305
# SCU_URL = scuxl0331
 SCU_URL = scuxl0162   # with MIL and ADDAC
# SCU_URL = scuxl0336  # DIOB-test
 # ACU-test

DOX_EXTRACT_STATIC         = "YES"
DOX_EXTRACT_PRIVATE = "YES"
# DOX_EXTRACT_ALL            = "YES"
DOX_MACRO_EXPANSION        = "YES"
DOX_EXPAND_ONLY_PREDEF     = "YES"
DOX_EXPAND_AS_DEFINED      = "FSM_DECLARE_STATE __DAQ_SHARED_IO_T"
DOX_EXPAND_AS_DEFINED      += ADD_NAMESPACE
DOX_EXPAND_AS_DEFINED      += ATOMIC_SECTION
DOX_DOTFILE_DIRS           += $(SCU_DIR)
ifdef ADDAC_DAQ
 DOX_DOTFILE_DIRS          += $(DAQ_LM32_DIR)
endif

ifeq ($(shell echo $${HOSTNAME:0:5}), asl74)
   DOX_OUTPUT_DIRECTORY = /common/usr/cscofe/doc/scu/lm32-firmware
endif
REPOSITORY_DIR := $(shell git rev-parse --show-toplevel)
# Including common makefile for arbitrary LM32 software for SCU
include $(REPOSITORY_DIR)/makefiles/makefile.scu

TESTFILE       ?= sinus-test.txt
TARGETTEST_DIR ?= /
SLOT           ?= 4
FG_NUM         ?= 0

FG_UT = fg-$(SLOT)-$(FG_NUM)

.PHONY: key
key:
	ssh-copy-id -i $(HOME)/.ssh/id_rsa.pub root@$(SCU_URL)

.PHONY: cptest
cptest:
	scp $(SCU_DIR)/$(TESTFILE) root@$(SCU_URL):$(TARGETTEST_DIR)

.PHONY: runfg
runfg:
	ssh root@$(SCU_URL) "(saft-fg-ctl  -r -f $(FG_UT) -g  <$(TARGETTEST_DIR)$(TESTFILE))"

.PHONY: stop
stop:
	ssh root@$(SCU_URL) "killall saft-fg-ctl"

.PHONY: scan
scan:
	ssh root@$(SCU_URL) "saft-fg-ctl -si"

#=================================== EOF ======================================
