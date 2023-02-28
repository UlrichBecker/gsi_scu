
MIAN_MODULE := lm32memtest.c

CODE_OPTIMIZATION = s

SOURCE += $(SCU_LIB_SRC_LM32_DIR)/scu_std_init.c
SOURCE += $(SCU_LIB_SRC_LM32_DIR)/scu_ddr3_lm32.c


DEFINES += CONFIG_SCU_USE_DDR3
#DEFINES += DEBUGLEVEL=1

# NO_LTO := 1

REPOSITORY_DIR = $(shell git rev-parse --show-toplevel)
include $(REPOSITORY_DIR)/makefiles/makefile.scu
#=================================== EOF ======================================
