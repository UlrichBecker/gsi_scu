###############################################################################
##                                                                           ##
##  Common Makefile LM32 FG software for SCU 3 and SCU 4 using FreeRTOS      ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:    scu_control_os.mk                                                ##
## (c):     GSI Helmholtz Centre for Heavy Ion Research GmbH                 ##
## Author:  Ulrich Becker                                                    ##
## Date:    12.10.2023                                                       ##
###############################################################################
MIAN_MODULE = $(BASE_DIR)/scu_control_os.c

ifdef ADDAC_DAQ
  SOURCE += $(BASE_DIR)/scu_task_daq.c
endif
ifdef SCU_MIL
  SOURCE += $(BASE_DIR)/scu_task_mil.c
endif
ifdef USE_ADDAC_FG_TASK
  SOURCE   += $(BASE_DIR)/scu_task_fg.c
  DEFINES  += CONFIG_USE_ADDAC_FG_TASK
endif
SOURCE   += $(SCU_LIB_SRC_LM32_DIR)/sys/ros_mutex.c


ifdef USE_TEMPERATURE_WATCHER
  SOURCE    += $(BASE_DIR)/scu_task_temperature.c
  DEFINES   += CONFIG_USE_TEMPERATURE_WATCHER
endif
SOURCE   += $(SCU_LIB_SRC_LM32_DIR)/ros_timeout.c

USE_LM32LOG=1
USE_MMU := 1
#------------------------------------------------------------------------------
USE_RTOS = 1

#
# Heap model 2 is needed, because child processes will dynamically created
# and destroyed.
#
RTOS_USING_HEAP = 4

# Enables the assert-macros within FreeRTOS-kernel.
# DEFINES += CONFIG_RTOS_PEDANTIC_CHECK
 
#------------------------------------------------------------------------------

#DEFINES += CONFIG_DISABLE_CRITICAL_SECTION
DEFINES += MAX_LM32_INTERRUPTS=2
# DEFINES += CONFIG_INTERRUPT_PEDANTIC_CHECK

LD_FLAGS += --specs=nosys.specs -ffreestanding -nostdlib

#NO_LTO := 1

DOX_SKIP_FUNCTION_MACROS = "NO"
DOX_MACRO_EXPANSION = "YES"

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
	ssh-copy-id -f -i $(HOME)/.ssh/id_rsa.pub root@$(SCU_URL)

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

