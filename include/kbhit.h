#pragma once


// See: http://cc.byexamples.com/2007/04/08/non-blocking-user-input-in-loop-without-ncurses/
int kbhit()
{
    struct timeval tv;
    fd_set fds;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds); //STDIN_FILENO is 0
    select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
    return FD_ISSET(STDIN_FILENO, &fds);
}

bool checkKbd( char ch = 0)
{
	if( ch == 0 && kbhit() ) {
		char ch = fgetc(stdin);
	}

	if( ch == 'q' || ch == 'Q') return true;

	return false;
}
