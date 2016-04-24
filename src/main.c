#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

#include "libwt/pty.h"

int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv;

    int _;
    struct pty pty;

    _ = forkpty(&pty);
    if (_ == 0)
    {
        /* TODO: figure out what to do with this
         * Supposedly the exec'd process would set the terminal to the mode it needs.
         * Should we provide a default?
         * Need to check how other terms do it.
         */
        struct termios term_settings;
        tcgetattr(pty.pts, &term_settings);
        cfmakeraw(&term_settings);
        tcsetattr(pty.pts, TCSANOW, &term_settings);

        char *av[] = { "bash", NULL };
        execvp(av[0], av);
        perror("execvp");
        return EXIT_FAILURE;
    }
    else if (_ < 0)
    {
        return EXIT_FAILURE;
    }

    fd_set fd_in;
    while (1)
    {
        siginfo_t status;
        status.si_pid = 0;
        waitid(P_PID, pty.child_pid, &status, WNOHANG | WEXITED);
        if (status.si_pid)
            break;

        FD_ZERO(&fd_in);
        FD_SET(STDIN_FILENO, &fd_in);
        FD_SET(pty.ptm, &fd_in);
        select(pty.ptm + 1, &fd_in, NULL, NULL, NULL);
        if (FD_ISSET(0, &fd_in))
        {
            char buf[1024];
            ssize_t read_bytes = read(0, buf, 1024);
            if (read_bytes > 0)
                write(pty.ptm, buf, read_bytes);
        }

        if (FD_ISSET(pty.ptm, &fd_in))
        {
            char buf[1024];
            ssize_t read_bytes = read(pty.ptm, buf, 1024);
            if (read_bytes > 0)
                write(STDOUT_FILENO, buf, read_bytes);
        }
    }
    return EXIT_SUCCESS;
}
