/*  From the Liunx 3.2.52 ChangeLog
    perf: Use css_tryget() to avoid propping up css refcount
    Signed-off-by: Salman Qazi <sqazi@google.com>

    commit 9c5da09d266ca9b32eb16cf940f8161d949c2fe5 upstream.

    An rmdir pushes css's ref count to zero.  However, if the associated
    directory is open at the time, the dentry ref count is non-zero.  If
    the fd for this directory is then passed into perf_event_open, it
    does a css_get().  This bounces the ref count back up from zero.  This
    is a problem by itself.  But what makes it turn into a crash is the
    fact that we end up doing an extra dput, since we perform a dput
    when css_put sees the ref count go down to zero.

    css_tryget() does not fall into that trap. So, we use that instead.

    Reproduction test-case for the bug:
*/

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/unistd.h>
#include <linux/perf_event.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#include "perf_helpers.h"

//#define PERF_FLAG_PID_CGROUP    (1U << 2)

     /*
      * Directly poke at the perf_event bug, since it's proving hard to repro
      * depending on where in the kernel tree.  what moved?
      */

int main(int argc, char **argv) {

	int fd;
	struct perf_event_attr attr;

	memset(&attr, 0, sizeof(attr));
	attr.exclude_kernel = 1;
	attr.size = sizeof(attr);
	mkdir("/dev/cgroup/perf_event/blah", 0777);
	fd = open("/dev/cgroup/perf_event/blah", O_RDONLY);
	perror("open");
	rmdir("/dev/cgroup/perf_event/blah");
	sleep(2);

	perf_event_open(&attr, fd, 0, -1,  PERF_FLAG_PID_CGROUP);
	perror("perf_event_open");
	close(fd);

	return 0;
}

