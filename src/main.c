#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

#include "aqueue.h"
#include "coro.h"
#include "log.h"
#include "pty.h"
#include "screen.h"
#include "unicode.h"

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

    struct unidecode *ud = unidecode_new(8192);
    struct screen *s = screen_new(80, 24, 1024);

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
            char buf[4096];
            ssize_t read_bytes = read(0, buf, 4096);
            if (read_bytes > 0)
                write(pty.ptm, buf, (unsigned) read_bytes);
        }

        if (FD_ISSET(pty.ptm, &fd_in))
        {
            const uint32_t *utf32;
            uint8_t out[4];
            if (aqueue_read(ud->read_queue, pty.ptm, 4096))
            {
                coro_resume(ud->coro);
                while (aqueue_empty(ud->out_queue) == 0)
                {
                    utf32 = aqueue_pop(ud->out_queue);
                    int spit = utf32_to_utf8(*utf32, out);
                    screen_putchar(s, *utf32);
                    write(STDOUT_FILENO, out, (size_t) spit);
                }
            }
        }
    }

    tcsetattr(0, TCSANOW, &term_settings_old);

    screen_free(s);
    unidecode_free(ud);

    return EXIT_SUCCESS;
}
