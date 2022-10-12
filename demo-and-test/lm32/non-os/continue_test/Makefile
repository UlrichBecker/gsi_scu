MIAN_MODULE := continue_test.c

REPOSITORY_DIR := $(shell git rev-parse --show-toplevel)

 #  TOOLCHAIN_DIR = $(INCLUDED_TOOLCHAIN_DIR)

 
SCU_DIR      = $(PRJ_DIR)/scu-control
DAQ_DIR      = $(SCU_DIR)/daq
DAQ_LM32_DIR = $(DAQ_DIR)/lm32

INCLUDE_DIRS += $(DAQ_DIR)
 
SOURCE += $(DAQ_LM32_DIR)/daq.c
SOURCE += $(SCU_LIB_SRC_LM32_DIR)/scu_bus.c
SOURCE += $(SCU_LIB_SRC_DIR)/dow_crc.c
SOURCE += $(WR_DIR)/w1-temp.c

#SOURCE += $(SCU_LIB_SRC_DIR)/aux.c
#SOURCE += $(SCU_LIB_SRC_DIR)/irq.c

DEFINES += CONFIG_DAQ_SINGLE_APP

DEFINES += CONFIG_DAQ_PEDANTIC_CHECK
DEFINES += CONFIG_SCU_BUS_PEDANTIC_CHECK
DEFINES += CONFIG_DAQ_DEBUG
#DEFINES += CONFIG_NO_SCU_ASSERT

#DEFINES += CONFIG_DAQ_SIMULATE_CHANNEL

CODE_OPTIMIZATION = s
# CFLAGS += -fstack-protector

include $(REPOSITORY_DIR)/makefiles/makefile.scu
#=================================== EOF ======================================

