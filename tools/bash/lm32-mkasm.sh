#!/bin/sh
###############################################################################
##                                                                           ##
##       Script to convert a C-source file in a LM32- assembler file         ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:      lm32-mksam.sh                                                  ##
## Author:    Ulrich Becker                                                  ##
## Date:      14.09.2022                                                     ##
## Copyright: GSI Helmholtz Centre for Heavy Ion Research GmbH               ##
###############################################################################
OPT=$2
if [ ! -n "$OPT" ]
then
   OPT=1
fi

lm32-elf-gcc --version
lm32-elf-gcc -O$OPT -S $(make lsrc | grep $1 ) $(make incargs) $(make devargs)

#=================================== EOF ======================================
