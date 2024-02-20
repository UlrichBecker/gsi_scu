################################################################################
##                                                                            ##
##            Makefile for  building a simple QT-application                  ##
##                                                                            ##
##----------------------------------------------------------------------------##
## File:   bel_projects/modules/linuxTest/qt_test/Makefile                    ##
## Author: Ulrich Becker                                                      ##
## (C)     2020 GSI Helmholtz Centre for Heavy Ion Research GmbH              ##
## Date:   25.08.2020                                                         ##
################################################################################
REPOSITORY_DIR = $(shell git rev-parse --show-toplevel)

MIAN_MODULE := qttest.cpp
SOURCE += simpleForm.cpp
FORMS += simpleForm.ui
DEFINES += CONFIG_DEBUG_MESSAGES

#GEN_SOURCE += $(MOC_PREFIX)simpleFrom.cpp

INCLUDE_DIRS += $(SCU_LIB_SRC_LINUX_DIR)

OPT_CPPCHECK += -D__LITTLE_ENDIAN__ -DQT_BOOTSTRAPPED -DQ_DECL_CONST_FUNCTION -DQPIXELFORMAT_H

include $(REPOSITORY_DIR)/makefiles/makefile.qt

$(UI_PREFIX)simpleForm.hpp: simpleForm.ui $(GENERATED_DIR)
	$(UIC_F) $< -o $@

$(MOC_PREFIX)simpleFrom.cpp: ./simpleForm.hpp $(UI_PREFIX)simpleForm.hpp
	$(MOC_F) $< -o $@

p:
	@echo $(MOC_SOURCE)
	@echo $(GEN_SOURCE)
	@echo $(FORMS_HEADER)

#=================================== EOF ======================================
