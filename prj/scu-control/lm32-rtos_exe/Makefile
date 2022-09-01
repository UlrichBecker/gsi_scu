###############################################################################
##                                                                           ##
##         Makefile LM32 FG software scu_control.bin using FreeRTOS          ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:    Makefile                                                         ##
## (c):     GSI Helmholtz Centre for Heavy Ion Research GmbH                 ##
## Author:  Ulrich Becker                                                    ##
## Date:    18.08.2022                                                       ##
###############################################################################
BASE_VERSION = 4
SUB_VERSION  = 6.0

#------------------------------------------------------------------------------
# USE_HISTORY=1
 USE_LM32LOG=1
 USE_MMU := 1
 SCU_MIL := 1
# MIL_GAP := 1

 MIL_DAQ_USE_DDR3 := 1
 ADDAC_DAQ := 1
 DIOB_WITH_DAQ := 1
# DEFINES += _CONFIG_DBG_MIL_TASK

NEW_ADDAC_HANDSHAKE := 1

#------------------------------------------------------------------------------
USE_RTOS = 1

#
# Heap model 2 is needed, because child processes will dynamically created
# and destroyed.
#
RTOS_USING_HEAP = 4

# Enables the assert-macros within FreeRTOS-kernel.
 DEFINES += CONFIG_RTOS_PEDANTIC_CHECK
 DEFINES += CONFIG_INTERRUPT_PEDANTIC_CHECK

#------------------------------------------------------------------------------

DEFINES += MAX_LM32_INTERRUPTS=2

# DEFINES += DEBUGLEVEL
MIAN_MODULE    = scu_control_os.c

ifdef ADDAC_DAQ
  SOURCE += scu_task_daq.c
endif
ifdef SCU_MIL
  SOURCE += scu_task_mil.c
endif
SOURCE   += scu_task_fg.c

include ../common_make.mk

# NO_LTO=1
 STACK_SIZE  =  512

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
