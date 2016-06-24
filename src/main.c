#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

#include "libwt/log.h"
#include "libwt/pty.h"

int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv;

    log_start(stderr, DEBUG);

    int rc;
    struct pty pty;

    rc = forkpty(&pty);
    if (rc == 0)
    {
        char *av[] = { "bash", NULL };
        execvp(av[0], av);
        errlog(ERROR, "execvp");
        return EXIT_FAILURE;
    }
    else if (rc < 0)
    {
        return EXIT_FAILURE;
    }

    struct termios term_settings, term_settings_old;
    tcgetattr(0, &term_settings_old);
    term_settings = term_settings_old;
    cfmakeraw(&term_settings);
    tcsetattr(0, TCSANOW, &term_settings);

    fd_set fd_in;
    while (1)
    {
        siginfo_t status;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdisabled-macro-expansion"
        status.si_pid = 0;
        waitid(P_PID, (unsigned) pty.child_pid, &status, WNOHANG | WEXITED);
        if (status.si_pid)
            break;
#pragma clang diagnostic pop

        FD_ZERO(&fd_in);
        FD_SET(STDIN_FILENO, &fd_in);
        FD_SET(pty.ptm, &fd_in);
        select(pty.ptm + 1, &fd_in, NULL, NULL, NULL);
        if (FD_ISSET(0, &fd_in))
        {
            char buf[1024];
            ssize_t read_bytes = read(0, buf, 1024);
            if (read_bytes > 0)
                write(pty.ptm, buf, (unsigned) read_bytes);
        }

        if (FD_ISSET(pty.ptm, &fd_in))
        {
            char buf[1024];
            ssize_t read_bytes = read(pty.ptm, buf, 1024);
            if (read_bytes > 0)
                write(STDOUT_FILENO, buf, (unsigned) read_bytes);
        }
    }

    tcsetattr(0, TCSANOW, &term_settings_old);
    return EXIT_SUCCESS;
}
