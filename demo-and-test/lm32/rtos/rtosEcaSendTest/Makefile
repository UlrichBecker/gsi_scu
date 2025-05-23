###############################################################################
##                                                                           ##
##   Makefile for FreeRTOS with sending an ECA-event to itself on SCU-LM32   ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:    FreeRTOS/Demo/SCU_LM32_GCC/rtosEcaSendTest/Makefile              ##
## (c):     GSI Helmholtz Centre for Heavy Ion Research GmbH                 ##
## Author:  Ulrich Becker                                                    ##
## Date:    07.11.2024                                                       ##
###############################################################################
MIAN_MODULE := rtosEcaSendTest.c

# The following flag is essential so that the build system adds the
# FreeRTOS sources to this project.
USE_RTOS = 1

# Additional module(s)
SOURCE += $(SCU_LIB_SRC_LM32_DIR)/scu_std_init.c
SOURCE += $(SCU_LIB_SRC_LM32_DIR)/eca_queue_type.c
SOURCE += $(SCU_LIB_SRC_LM32_DIR)/sys/ros_mutex.c

RTOS_USING_HEAP = 1

# TOOLCHAIN_DIR = $(INCLUDED_TOOLCHAIN_DIR)

CODE_OPTIMIZATION = s
# NO_LTO=1

# When the following compiler switch is defined, then the assert-macros
# within the FreeRTOS kernel will activated.
# Meaningful in debug mode.
# DEFINES += CONFIG_RTOS_PEDANTIC_CHECK
# DEFINES += CONFIG_INTERRUPT_PEDANTIC_CHECK
# DEFINES += CONFIG_IRQ_ENABLING_IN_ATOMIC_SECTIONS
# DEFINES += CONFIG_DISABLE_CRITICAL_SECTION

# DEFINES += CONFIG_IRQ_RESET_IP_AFTER
# Only interrupt 0 and 1 will be used therefore the interrupt vector table
# can be reduced form the maximum of 32 to two items.
DEFINES += MAX_LM32_INTERRUPTS=2

# FreeRTOS will handle the stack defined in FreeRTOSConfig.h.
# Therefore we can overwrite the default size of the base stack size (10k)
# into a smaller size.
STACK_SIZE := 256

REPOSITORY_DIR = $(shell git rev-parse --show-toplevel)
include $(REPOSITORY_DIR)/makefiles/makefile.scu
#=================================== EOF ======================================
