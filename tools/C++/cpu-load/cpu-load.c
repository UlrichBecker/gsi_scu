 
#include <stdbool.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
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
    printf("Press any key to end...\n");
    while( !kbhit() ) {}
    printf("End...\n");
    return 0;
}

/*================================== EOF ====================================*/
