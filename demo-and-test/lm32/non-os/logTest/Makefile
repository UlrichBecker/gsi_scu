
MIAN_MODULE := logtest.c

# TOOLCHAIN_DIR = $(INCLUDED_TOOLCHAIN_DIR)

CODE_OPTIMIZATION = 2

SOURCE += $(SCU_LIB_SRC_LM32_DIR)/scu_std_init.c
SOURCE += $(SCU_LIB_SRC_LM32_DIR)/lm32_syslog.c
SOURCE += $(SCU_LIB_SRC_LM32_DIR)/scu_ddr3_lm32.c
SOURCE += $(SCU_LIB_SRC_DIR)/scu_mmu.c
SOURCE += $(SCU_LIB_SRC_LM32_DIR)/scu_mmu_lm32.c
SOURCE += $(SCU_LIB_SRC_LM32_DIR)/sys/lm32Interrupts.c
SOURCE += $(SCU_LIB_SRC_DIR)/fifo/circular_index.c

DEFINES += CONFIG_SCU_USE_DDR3
#DEFINES += DEBUGLEVEL=1
DEFINES += CONFIG_LOG_TEST

# DEFINES += CONFIG_CIRCULAR_DEBUG

# NO_LTO := 1

REPOSITORY_DIR = $(shell git rev-parse --show-toplevel)
include $(REPOSITORY_DIR)/makefiles/makefile.scu
#=================================== EOF ======================================
