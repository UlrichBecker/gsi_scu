/*!
 *  @file cpu-load.c
 *  @brief This program does nothing but load the CPU for testing purposes.
 *
 *  @date 02.11.2023
 *  @copyright (C) 2023 GSI Helmholtz Centre for Heavy Ion Research GmbH
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
#include <stdbool.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

bool kbhit( void )
{
    struct termios oldTerm, newTerm;
    int ch;
    int oldf;

    tcgetattr( STDIN_FILENO, &oldTerm );
    newTerm = oldTerm;
    newTerm.c_lflag &= ~(ICANON | ECHO);
    tcsetattr( STDIN_FILENO, TCSANOW, &newTerm );
    oldf = fcntl( STDIN_FILENO, F_GETFL, 0 );
    fcntl( STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK );

    ch = getchar();

    tcsetattr( STDIN_FILENO, TCSANOW, &oldTerm );
    fcntl( STDIN_FILENO, F_SETFL, oldf );

    if( ch != EOF )
    {
        ungetc( ch, stdin );
        return true;
    }

    return false;
}

int main( void )
{
    printf( "This program does nothing but load the CPU for testing purposes.\n" );
    printf( "Press any key to end...\n" );
    while( !kbhit() ) {}
    printf( "End...\n" );
    return EXIT_SUCCESS;
}

/*================================== EOF ====================================*/
