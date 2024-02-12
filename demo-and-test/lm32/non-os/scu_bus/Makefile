MIAN_MODULE := scu_bus_test.c
SOURCE += $(SCU_LIB_SRC_LM32_DIR)/scu_std_init.c
SOURCE += $(SCU_LIB_SRC_LM32_DIR)/scu_bus.c
SOURCE += $(SCU_LIB_SRC_LM32_DIR)/sys/lm32Interrupts.c

DEFINES += CONFIG_SCU_BUS_PEDANTIC_CHECK

REPOSITORY_DIR := $(shell git rev-parse --show-toplevel)
include $(REPOSITORY_DIR)/makefiles/makefile.scu
#=================================== EOF ======================================

