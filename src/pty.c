#include <fcntl.h>
#include <stdlib.h>
#include <sys/ioctl.h>

#include "log.h"
#include "pty.h"

int openpty(struct pty *pty)
{
    pty->child_pid = 0;

    pty->ptm = posix_openpt(O_RDWR);
    if (pty->ptm < 0)
    {
        errlog(ERROR, "Failed to open PTY master");
        goto fail;
    }

    grantpt(pty->ptm);
    unlockpt(pty->ptm);

    pty->pts = open(ptsname(pty->ptm), O_RDWR);
    if (pty->pts < 0)
    {
        errlog(ERROR, "Failed to open PTY slave");
        goto close_ptm;
    }

    return 0;

close_ptm:
    close(pty->ptm);
fail:
    return -1;
}

int forkpty(struct pty *pty)
{
    int rc;

    rc = openpty(pty);
    if (rc < 0)
        goto fail;

    pty->child_pid = fork();
    if (pty->child_pid == 0)
    {
        close(pty->ptm);
        pty->ptm = -1;

        dup2(pty->pts, STDIN_FILENO);
        dup2(pty->pts, STDOUT_FILENO);
        dup2(pty->pts, STDERR_FILENO);

        close(pty->pts);

        setsid();

        ioctl(STDIN_FILENO, TIOCSCTTY, 1);

        return 0;
    }
    else if (pty->child_pid > 0)
    {
        close(pty->pts);
        pty->pts = -1;
        return 1;
    }
    else
    {
        errlog(ERROR, "fork");
        goto close_pty;
    }

close_pty:
    closepty(pty);
fail:
    return -1;
}

void closepty(struct pty *pty)
{
    if (pty->ptm >= 0)
    {
        close(pty->ptm);
        pty->ptm = -1;
    }

    if (pty->pts >= 0)
    {
        close(pty->pts);
        pty->pts = -1;
    }
}
