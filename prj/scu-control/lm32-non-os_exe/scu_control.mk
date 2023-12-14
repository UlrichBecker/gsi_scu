###############################################################################
##                                                                           ##
##     Common Makefile LM32 FG software for SCU 3 and SCU 4 without OS       ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:    scu_control.mk                                                   ##
## (c):     GSI Helmholtz Centre for Heavy Ion Research GmbH                 ##
## Author:  Ulrich Becker                                                    ##
## Date:    12.10.2023                                                       ##
###############################################################################
#------------------------------------------------------------------------------
# USE_HISTORY=1
 USE_LM32LOG=1
 USE_MMU := 1
 SCU_MIL := 1
# MIL_GAP := 1

# MIL_DAQ_USE_RAM := 1

ifdef SCU_MIL
 MIL_IN_TIMER_INTERRUPT := 1
endif
 ADDAC_DAQ := 1
 DIOB_WITH_DAQ := 1
# DEFINES += _CONFIG_DBG_MIL_TASK
#------------------------------------------------------------------------------




# DEFINES += DEBUGLEVEL
MIAN_MODULE    = $(BASE_DIR)/scu_main.c

SOURCE += $(SCU_LIB_SRC_LM32_DIR)/sys/lm32Interrupts.c
ifdef ADDAC_DAQ
  SOURCE += $(DAQ_LM32_DIR)/daq_main.c
endif
SOURCE += $(WR_DIR)/usleep.c
SOURCE += $(WR_DIR)/syscon.c

ifdef MIL_IN_TIMER_INTERRUPT
 DEFINES += MAX_LM32_INTERRUPTS=2
 DEFINES += CONFIG_MIL_IN_TIMER_INTERRUPT
else
 DEFINES += MAX_LM32_INTERRUPTS=1
endif

#DEFINES += CONFIG_INTERRUPT_PEDANTIC_CHECK
#DEFINES += CONFIG_IRQ_RESET_IP_AFTER

LD_FLAGS += --specs=nosys.specs -ffreestanding -nostdlib

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

INCLUDE_DIRS += $(BASE_DIR)/
GENERATED_DIR = $(shell pwd)/generated

include $(BASE_DIR)/../common_make.mk


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

