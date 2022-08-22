/*!
 * @file scu_task_mil.c
 * @brief FreeRTOS task for MIL function generators and MIL DAQs.
 *
 * @date 22.08.2022
 * @copyright (C) 2022 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 * @author Ulrich Becker <u.becker@gsi.de>
 *
 * @see scu_task_mil.h
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
#include <FreeRTOS.h>
#include <task.h>
#include "scu_task_mil.h"


STATIC TaskHandle_t mg_taskMilHandle = NULL;

STATIC void taskMil( void* pTaskData UNUSED )
{
   taskInfoLog();

   while( true )
   {
   }
}

void taskStartMil( void )
{
   TASK_CREATE_OR_DIE( taskMil, 1024, 1, &mg_taskMilHandle );
}

void taskStopMil( void )
{
}

/*================================== EOF ====================================*/
