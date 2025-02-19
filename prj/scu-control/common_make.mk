###############################################################################
##                                                                           ##
##   Common makefile LM32 FG software scu_control.bin for os and non os      ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:    common_make.mk                                                   ##
## (c):     GSI Helmholtz Centre for Heavy Ion Research GmbH                 ##
## Author:  Ulrich Becker                                                    ##
## Date:    09.01.2022                                                       ##
###############################################################################

NEW_ADDAC_HANDSHAKE := 1

ifdef MIL_DAQ_USE_RAM
   DEFINES += CONFIG_MIL_DAQ_USE_RAM
   USE_SCU_EXTERN_RAM := 1
endif

DEFINES += CONFIG_COUNT_MSI_PER_IRQ

# DEFINES += CONFIG_USE_SENT_COUNTER
# DEFINES += CONFIG_IRQ_ENABLING_IN_ATOMIC_SECTIONS
# DEFINES += CONFIG_NO_LM32_ASSERT
# DEFINES += CONFIG_PRINT_BUFSIZE=128 #!!
# DEFINES += CONFIG_PRINTF_64BIT      #!!
#DEFINES += CONFIG_ASSERT
DEFINES += CONFIG_HANDLE_UNKNOWN_MSI
DEFINES += SDBFS_BIG_ENDIAN
DEFINES += CONFIG_WR_NODE
DEFINES += CONFIG_RESET_QUEUE_IF_OVERFLOW

DEFINES += CONFIG_DDR3_NO_BURST_FUNCTIONS
DEFINES += CONFIG_NON_DAQ_FG_SUPPORT

 # DEFINES += CONFIG_USE_INTERRUPT_TIMESTAMP
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


 DEFINES += CONFIG_USE_INTERRUPT_TIMESTAMP
# DEFINES += CONFIG_IRQ_RESET_IP_AFTER
# DEFINES += CONFIG_DISABLE_CRITICAL_SECTION
 DEFINES +=  CONFIG_IRQ_ENABLING_IN_ATOMIC_SECTIONS

# DEFINES += MICO32_FULL_CONTEXT_SAVE_RESTORE

# DEFINES += DEBUG_SAFTLIB
ifdef SCU_MIL
  DEFINES += CONFIG_MIL_FG
 ifdef SCU3
   DEFINES += CONFIG_MIL_PIGGY
 endif
 ifdef MIL_DAQ_USE_RAM
  VERSION_STR += "+MIL-DDR3"
 else
  VERSION_STR += "+MIL"
 endif
 ifdef MIL_IN_TIMER_INTERRUPT
  VERSION_STR += "+TI"
 endif
 ifdef MIL_GAP
  DEFINES += CONFIG_READ_MIL_TIME_GAP
  DEFINES += _CONFIG_VARIABLE_MIL_GAP_READING
  VERSION_STR += "+GAP"
 endif
endif
ifdef USE_LM32LOG
  DEFINES += CONFIG_USE_LM32LOG
  VERSION_STR += "+LOG"
endif
ifdef ADDAC_DAQ
  DEFINES += CONFIG_SCU_DAQ_INTEGRATION
  DEFINES += CONFIG_DAQ_SW_SEQUENCE
  DEFINES += DAQ_MAX_CHANNELS=4
  VERSION_STR += "+DAQ"
  ifdef DIOB_WITH_DAQ
     DEFINES += CONFIG_DIOB_WITH_DAQ
     VERSION_STR += "+DIOB-DAQ"
  endif
endif
ifdef USE_ADDAC_FG_TASK
  VERSION_STR += "+AFGT"
else
  VERSION_STR += "+AFGI"
endif
VERSION = $(VERSION_STR)

SCU_DIR      = $(PRJ_DIR)/scu-control
DAQ_DIR      = $(SCU_DIR)/daq
DAQ_LM32_DIR = $(DAQ_DIR)/lm32

SOURCE += $(SCU_LIB_SRC_LM32_DIR)/scu_std_init.c
SOURCE += $(SCU_LIB_SRC_LM32_DIR)/scu_logutil.c
SOURCE += $(SCU_LIB_SRC_LM32_DIR)/scu_mailbox.c
SOURCE += $(SCU_LIB_SRC_LM32_DIR)/scu_bus.c
SOURCE += $(SCU_LIB_SRC_LM32_DIR)/event_measurement.c
SOURCE += $(SCU_LIB_SRC_DIR)/fifo/circular_index.c
SOURCE += $(SCU_LIB_SRC_DIR)/fifo/sw_queue.c
SOURCE += $(SCU_DIR)/scu_lm32_common.c
SOURCE += $(SCU_DIR)/queue_watcher.c
SOURCE += $(SCU_DIR)/sys_exception.c
SOURCE += $(SCU_DIR)/fg/scu_fg_list.c
ifndef MIL_DAQ_USE_RAM
   SOURCE += $(SCU_DIR)/fg/scu_circular_buffer.c
endif
SOURCE += $(SCU_DIR)/fg/scu_fg_macros.c
SOURCE += $(SCU_DIR)/fg/scu_fg_handler.c
SOURCE += $(SCU_DIR)/scu_command_handler.c
SOURCE += $(SCU_DIR)/temperature/scu_temperature.c
ifdef SCU_MIL
  SOURCE += $(SCU_LIB_SRC_LM32_DIR)/eca_queue_type.c
  SOURCE += $(SCU_DIR)/fg/scu_eca_handler.c
  SOURCE += $(SCU_LIB_SRC_LM32_DIR)/scu_mil.c
  SOURCE += $(SCU_DIR)/fg/scu_mil_fg_handler.c
  SOURCE += $(SCU_LIB_SRC_DIR)/fifo/scu_event.c
endif
ifdef ADDAC_DAQ
  SOURCE += $(SCU_LIB_SRC_LM32_DIR)/scu_ddr3_lm32.c
  SOURCE += $(DAQ_LM32_DIR)/daq.c
  SOURCE += $(DAQ_DIR)/daq_fg_allocator.c
  SOURCE += $(DAQ_LM32_DIR)/daq_fg_switch.c
  SOURCE += $(DAQ_LM32_DIR)/daq_command_interface_uc.c
  SOURCE += $(DAQ_LM32_DIR)/daq_ramBuffer_lm32.c
  USE_SCU_EXTERN_RAM := 1
endif
ifdef USE_LM32LOG
  SOURCE += $(SCU_LIB_SRC_LM32_DIR)/lm32_syslog.c
endif
ifdef USE_SCU_EXTERN_RAM
  DEFINES += CONFIG_SCU_USE_DDR3
endif
ifdef USE_MMU
  SOURCE += $(SCU_LIB_SRC_LM32_DIR)/scu_mmu_lm32.c
  SOURCE += $(SCU_LIB_SRC_DIR)/scu_mmu.c
  DEFINES += CONFIG_USE_MMU
  VERSION_STR += "+MMU"
endif

SOURCE += $(SCU_LIB_SRC_DIR)/dow_crc.c
SOURCE += $(WR_DIR)/w1-temp.c
SOURCE += $(WR_DIR)/w1.c
SOURCE += $(WR_DIR)/w1-hw.c

# ADDITIONAL_HEADDER_IN_BUILD_ID = $(SCU_DIR)/scu_shared_mem.h


# CODE_OPTIMIZATION = 2
 CODE_OPTIMIZATION = s
# CODE_OPTIMIZATION = 0
# TOOLCHAIN_DIR = $(INCLUDED_TOOLCHAIN_DIR)
# TOOLCHAIN_DIR = /home/bel/ubecker/lnx/src/extern/gcc-toolchain-builder/TEST93/bin/
# RAM_SIZE    = 147456
# NO_LTO := 1

ifdef ADDAC_DAQ
   ifdef MIL_DAQ_USE_RAM
    ifdef NEW_ADDAC_HANDSHAKE
      SHARED_SIZE = 24832
    else
      SHARED_SIZE = 24836
    endif
   else
    ifdef NEW_ADDAC_HANDSHAKE
      SHARED_SIZE = 65776
    else
      SHARED_SIZE = 65780
    endif
   endif
else
   SHARED_SIZE = 81920
endif
DOX_MACRO_EXPANSION = "YES"
DOX_EXTRACT_STATIC         = "YES"
DOX_EXTRACT_PRIVATE = "YES"
# DOX_EXTRACT_ALL            = "YES"
DOX_MACRO_EXPANSION        = "YES"
DOX_EXPAND_ONLY_PREDEF     = "YES"
DOX_EXPAND_AS_DEFINED      = "FSM_DECLARE_STATE __DAQ_SHARED_IO_T"
DOX_EXPAND_AS_DEFINED      += ADD_NAMESPACE
# DOX_EXPAND_AS_DEFINED      += ATOMIC_SECTION
# DOX_PREDEFINED += ATOMIC_SECTION()=
DOX_PREDEFINED += STATIC
DOX_PREDEFINED += criticalSectionEnter()=criticalSectionEnterBase()
#DOX_EXPAND_AS_DEFINED      += criticalSectionEnter
DOX_EXPAND_AS_DEFINED      += criticalSectionExit
DOX_DOTFILE_DIRS           += $(SCU_DIR)
DOX_SKIP_FUNCTION_MACROS   = "NO"
ifdef ADDAC_DAQ
 DOX_DOTFILE_DIRS          += $(DAQ_LM32_DIR)
endif

ifndef DOX_OUTPUT_DIRECTORY
 ifeq ($(shell echo $${HOSTNAME:0:5}), asl75)
   DOX_OUTPUT_DIRECTORY = /common/usr/cscofe/doc/scu/lm32-firmware/$(TARGET)
 endif
endif

REPOSITORY_DIR := $(shell git rev-parse --show-toplevel)
# Including common makefile for arbitrary LM32 software for SCU

include $(REPOSITORY_DIR)/makefiles/makefile.scu


.PHONY: testinstall
testinstall: $(BIN_FILE)
	cp $(BIN_FILE) /common/export/nfsinit/global/lm32-test-apps/

#=================================== EOF ======================================
