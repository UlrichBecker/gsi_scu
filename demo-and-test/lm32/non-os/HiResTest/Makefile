MIAN_MODULE := hirestest.c

SCU_DIR      = $(PRJ_DIR)/scu-control
DAQ_DIR      = $(SCU_DIR)/daq
DAQ_LM32_DIR = $(DAQ_DIR)/lm32

INCLUDE_DIRS += $(DAQ_DIR)


SOURCE += $(DAQ_LM32_DIR)/daq.c
SOURCE += $(SCU_LIB_SRC_LM32_DIR)/sys/lm32Interrupts.c
SOURCE += $(SCU_LIB_SRC_LM32_DIR)/scu_bus.c
 SOURCE += $(SCU_LIB_SRC_LM32_DIR)/scu_std_init.c
SOURCE += $(SCU_LIB_SRC_DIR)/dow_crc.c
SOURCE += $(SCU_LIB_SRC_DIR)/fifo/sw_queue.c
SOURCE += $(SCU_LIB_SRC_DIR)/fifo/circular_index.c
SOURCE += $(DAQ_DIR)/daq_fg_allocator.c
# SOURCE += $(WR_DIR)/w1-temp.c

DEFINES += CONFIG_DAQ_SINGLE_APP
DEFINES += CONFIG_DAQ_PEDANTIC_CHECK
DEFINES += CONFIG_SCU_BUS_PEDANTIC_CHECK
DEFINES += CONFIG_DAQ_DEBUG
CODE_OPTIMIZATION=0

REPOSITORY_DIR := $(shell git rev-parse --show-toplevel)
include $(REPOSITORY_DIR)/makefiles/makefile.scu
#=================================== EOF ======================================

