

REPOSITORY_DIR = $(shell git rev-parse --show-toplevel)

MIAN_MODULE := irqTimerTest.c

# TOOLCHAIN_DIR = $(INCLUDED_TOOLCHAIN_DIR)

CODE_OPTIMIZATION = s

SOURCE += $(SCU_LIB_SRC_LM32_DIR)/sys/lm32Interrupts.c
SOURCE += $(SCU_LIB_SRC_LM32_DIR)/scu_std_init.c
SOURCE += $(SCU_LIB_SRC_LM32_DIR)/event_measurement.c

DEFINES += CONFIG_INTERRUPT_PEDANTIC_CHECK
DEFINES += MAX_LM32_INTERRUPTS=2
DEFINES += CONFIG_SCU_ASSERT_CONTINUE

INCLUDE_DIRS   += $(RTOS_PORT_DIR)

# NO_LTO := 1

include $(REPOSITORY_DIR)/makefiles/makefile.scu
#=================================== EOF ======================================
