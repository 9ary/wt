#ifndef _PTY_H
#define _PTY_H

#include <unistd.h>

struct pty
{
    pid_t child_pid;
    int ptm, pts;
};


int forkpty(struct pty *pty);
int openpty(struct pty *pty);
void closepty(struct pty *pty);

#endif
