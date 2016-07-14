#include <errno.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>

int sp_forkexec(char **argv,
        pid_t *child_pid,
        int cldin,
        int cldout,
        int clderr,
        int ptm)
{
    int rc;

    *child_pid = fork();
    if (*child_pid == 0)
    {
        close(ptm);

        if (cldin >= 0)
        {
            rc = dup2(cldin, STDIN_FILENO);
            if (rc < 0)
                goto child_fail;
            if (ptm < 0)
                close(cldin);
        }

        if (cldout >= 0)
        {
            rc = dup2(cldout, STDOUT_FILENO);
            if (rc < 0)
                goto child_fail;
            // stdout and stderr may be the same, but stdin shouldn't be unless it's a term
            if (ptm < 0 && cldout != clderr)
                close(cldout);
        }

        if (clderr >= 0)
        {
            rc = dup2(clderr, STDERR_FILENO);
            if (rc < 0)
                goto child_fail;
            if (ptm < 0)
                close(clderr);
        }

        // If we're attaching to a PTY, extra work is required before we exec away
        if (ptm >= 0)
        {
            close(cldin);

            setsid();
            rc = ioctl(STDIN_FILENO, TIOCSCTTY, 1);
            if (rc < 0)
                goto child_fail;
        }

        execvp(argv[0], argv);

child_fail:
        exit(EXIT_FAILURE);
    }
    else if (*child_pid > 0)
    {
        if (ptm >= 0)
            close(cldin);
        return 0;
    }

    // Fork failed
    return errno;
}
