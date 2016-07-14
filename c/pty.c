#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

int openpty(int *ptm, int *pts)
{
    int rc;

    *ptm = posix_openpt(O_RDWR);
    if (*ptm < 0)
        goto fail;

    rc = grantpt(*ptm);
    if (rc < 0)
        goto close_ptm;

    rc = unlockpt(*ptm);
    if (rc < 0)
        goto close_ptm;

    *pts = open(ptsname(*ptm), O_RDWR);
    if (*pts < 0)
        goto close_ptm;

    return 0;

close_ptm:
    close(*ptm);
    *ptm = -1;
fail:
    return errno;
}
