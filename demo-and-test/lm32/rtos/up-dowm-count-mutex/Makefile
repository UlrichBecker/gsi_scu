###############################################################################
##                                                                           ##
##         Makefile for test application of FreeRTOS on SCU-LM32             ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:    FreeRTOS/Demo/SCU_LM32_GCC/up-down_count/Makefile                ##
## (c):     GSI Helmholtz Centre for Heavy Ion Research GmbH                 ##
## Author:  Ulrich Becker                                                    ##
## Date:    24.03.2020                                                       ##
###############################################################################
REPOSITORY_DIR = $(shell git rev-parse --show-toplevel)

MIAN_MODULE := up-down-count-mutex.c

# The following flag is essential so that the build system adds the
# FreeRTOS sources to this project.
 USE_RTOS = 1

RTOS_USING_HEAP = 1

# TOOLCHAIN_DIR = $(INCLUDED_TOOLCHAIN_DIR)

CODE_OPTIMIZATION = 1

SOURCE += $(SCU_LIB_SRC_LM32_DIR)/scu_std_init.c
SOURCE += $(SCU_LIB_SRC_LM32_DIR)/sys/ros_mutex.c

# When the following compiler switch is defined, then the assert-macros
# within the FreeRTOS kernel will activated.
# Meaningful in debug mode.
 DEFINES += CONFIG_RTOS_PEDANTIC_CHECK
 DEFINES += CONFIG_INTERRUPT_PEDANTIC_CHECK

DEFINES += MAX_LM32_INTERRUPTS=2

# FreeRTOS will handle the stack defined in FreeRTOSConfig.h.
# Therefore we can overwrite the default size of the base stack size (10k)
# into a smaller size.
STACK_SIZE := 256
DOX_REFERENCES_RELATION = YES
SCU_URL = scuxl0692
include $(REPOSITORY_DIR)/makefiles/makefile.scu
#=================================== EOF ======================================
