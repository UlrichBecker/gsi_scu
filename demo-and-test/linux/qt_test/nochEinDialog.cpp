/*!
 *  @file nochEinDialog.cpp
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
#include "nochEinDialog.hpp"
#include <message_macros.hpp>

NochEinDialogForm::NochEinDialogForm( QDialog* pDialog )
   :QDialog( pDialog )
{
   DEBUG_MESSAGE_M_FUNCTION("");
   m_oUi.setupUi( this );
}

NochEinDialogForm::~NochEinDialogForm( void )
{
   DEBUG_MESSAGE_M_FUNCTION("");
}

//================================== EOF ======================================
