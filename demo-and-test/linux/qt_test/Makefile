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


UI_PREFIX  = $(GENERATED_DIR)/ui_
MOC_PREFIX = $(GENERATED_DIR)/moc_

DEFINES += CONFIG_DEBUG_MESSAGES

SOURCE += $(MOC_PREFIX)simpleFrom.cpp

QTDIR += /usr/include/qt5
QTDIR += /usr/include/qt5/QtCore
QTDIR += /usr/include/qt5/QtWidgets

LIBS += Qt5Gui Qt5Core Qt5Widgets
LIBS += stdc++

INCLUDE_DIRS += $(QTDIR)
INCLUDE_DIRS += $(SCU_LIB_SRC_LINUX_DIR)

UIC ?= uic
MOC ?= moc
RCC ?= rcc

MOC_MSG := "MOC"
UIC_MSG := "UIC"
RCC_MSG := "RCC"

ifeq ($(V), 1)
   UIC_F = $(UIC) 
   MOC_F = $(MOC)
   RCC_F = $(RCC)
else
   UIC_F = $(FORMAT) "gen" $(UIC_MSG) $(@); $(UIC)
   MOC_F = $(FORMAT) "gen" $(MOC_MSG) $(@); $(MOC)
   RCC_F = $(FORMAT) "gen" $(RCC_MSG) $(@); $(RCC)
endif

ADDITIONAL_TO_CLEAN += $(GENERATED_DIR)/*.*

include $(REPOSITORY_DIR)/makefiles/makefile.scun

$(UI_PREFIX)simpleForm.hpp: simpleForm.ui $(GENERATED_DIR)
	$(UIC_F) $< -o $@

$(MOC_PREFIX)simpleFrom.cpp: ./simpleForm.hpp $(UI_PREFIX)simpleForm.hpp
	$(MOC_F) $< -o $@

#=================================== EOF ======================================
