/*!
 *  @file nochEinDialog.hpp
 *  @brief Simple QT- dialog box for testing building QT/KDE applications
 *
 *  @date 25.08.2020
 *  @copyright (C) 2020 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 *  @author Ulrich Becker <u.becker@gsi.de>
 *
 ******************************************************************************
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************
 */
#ifndef _NOCHEINDIALOG_HPP
#define _NOCHEINDIALOG_HPP

/*
 * uic nochEinDialog.ui  -o ./generated/ui_nochEinDialog.hpp
 * moc nochEinDialog.hpp -o ./generated/moc_nochEinDialog.cpp
 */
#include <QObject>
#include <generated/ui_nochEinDialog.hpp>

class NochEinDialogForm: public QDialog
{
   Q_OBJECT

   Ui::nochEinDialog m_oUi;
public:
   NochEinDialogForm( QDialog* = nullptr );
   ~NochEinDialogForm( void );
};

#endif // _NOCHEINDIALOG_HPP
//================================== EOG ======================================
