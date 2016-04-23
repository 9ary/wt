#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv;

    int ptm = posix_openpt(O_RDWR);
    grantpt(ptm);
    unlockpt(ptm);
    int pts = open(ptsname(ptm), O_RDWR);

    pid_t child = fork();
    if (child == 0)
    {
        close(ptm);
        struct termios term_settings;
        tcgetattr(pts, &term_settings);
        cfmakeraw(&term_settings);
        tcsetattr(pts, TCSANOW, &term_settings);
        dup2(pts, STDIN_FILENO);
        dup2(pts, STDOUT_FILENO);
        dup2(pts, STDERR_FILENO);
        close(pts);
        setsid();
        ioctl(0, TIOCSCTTY, 1);
        char *av[] = { "bash" };
        execvp(av[0], av);
        perror("execvp");
        return EXIT_FAILURE;
    }
    else if (child < 0)
    {
        return EXIT_FAILURE;
    }

    close(pts);

    fd_set fd_in;
    while (1)
    {
        siginfo_t status;
        status.si_pid = 0;
        waitid(P_PID, child, &status, WNOHANG | WEXITED);
        if (status.si_pid)
            break;

        FD_ZERO(&fd_in);
        FD_SET(0, &fd_in);
        FD_SET(ptm, &fd_in);
        select(ptm + 1, &fd_in, NULL, NULL, NULL);
        if (FD_ISSET(0, &fd_in))
        {
            char buf[1024];
            ssize_t read_bytes = read(0, buf, 1024);
            if (read_bytes > 0)
                write(ptm, buf, read_bytes);
        }

        if (FD_ISSET(ptm, &fd_in))
        {
            char buf[1024];
            ssize_t read_bytes = read(ptm, buf, 1024);
            if (read_bytes > 0)
                write(STDOUT_FILENO, buf, read_bytes);
        }
    }
    return EXIT_SUCCESS;
}
