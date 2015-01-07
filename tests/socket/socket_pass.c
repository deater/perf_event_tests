/* Test passing perf_event_open() fd over a socket	*/
/* Based on some socket passing code by Keith Packard	*/
/*	http://keithp.com/blogs/fd-passing/		*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include "perf_event.h"
#include "test_utils.h"
#include "perf_helpers.h"
#include "instructions_testcode.h"


static char test_string[]="Testing passing fd over a socket...";
static int quiet;

ssize_t sock_fd_write(int sock, void *buf, ssize_t buflen, int fd) {

	ssize_t size;
	struct msghdr msg;
	struct iovec iov;
	union {
		struct cmsghdr cmsghdr;
		char        control[CMSG_SPACE(sizeof (int))];
	} cmsgu;
	struct cmsghdr *cmsg;
	int *fd_location;

	iov.iov_base = buf;
	iov.iov_len = buflen;

	msg.msg_name = NULL;	/* optional address */
	msg.msg_namelen = 0;	/* size of address */
	msg.msg_iov = &iov;	/* scatter gather array */
	msg.msg_iovlen = 1;	/* elements in iov */
        msg.msg_control = cmsgu.control;	/* control data */
        msg.msg_controllen = sizeof(cmsgu.control);
				/* flags is unused */

	cmsg = CMSG_FIRSTHDR(&msg);
	cmsg->cmsg_len = CMSG_LEN(sizeof (int));
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type = SCM_RIGHTS;

	if (!quiet) printf ("passing fd %d\n", fd);
	fd_location = (int *) CMSG_DATA(cmsg);
	*fd_location=fd;

	size = sendmsg(sock, &msg, 0);

	if (size < 0) {
		perror ("sendmsg");
	}

	return size;
}

ssize_t sock_fd_read(int sock, void *buf, ssize_t bufsize, int *fd) {

	ssize_t     size;
	struct msghdr msg;
	struct iovec iov;
	union {
		struct cmsghdr  cmsghdr;
		char control[CMSG_SPACE(sizeof (int))];
	} cmsgu;
	struct cmsghdr  *cmsg;
	int *fd_location;

	iov.iov_base = buf;
	iov.iov_len = bufsize;

	msg.msg_name = NULL;
	msg.msg_namelen = 0;
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_control = cmsgu.control;
	msg.msg_controllen = sizeof(cmsgu.control);
	size = recvmsg (sock, &msg, 0);
	if (size < 0) {
		perror ("recvmsg");
		exit(1);
	}

	cmsg = CMSG_FIRSTHDR(&msg);
	if (cmsg && cmsg->cmsg_len == CMSG_LEN(sizeof(int))) {
		if (cmsg->cmsg_level != SOL_SOCKET) {
			fprintf (stderr, "invalid cmsg_level %d\n",
				cmsg->cmsg_level);
			exit(1);
		}
		if (cmsg->cmsg_type != SCM_RIGHTS) {
			fprintf (stderr, "invalid cmsg_type %d\n",
				cmsg->cmsg_type);
			exit(1);
		}

		fd_location=(int *) CMSG_DATA(cmsg);
		*fd = *fd_location; 

		if (!quiet) printf ("received fd %d\n", *fd);
	} else {
		*fd = -1;
	}
	return size;
}


void child(int sock) {

	int fd;
	char buf[16];
	ssize_t size;

	long long count[1];
	int result,read_result;

	sleep(1);

	for (;;) {
		/* read fd from socket */
		size = sock_fd_read(sock, buf, sizeof(buf), &fd);

	        if (size <= 0)
			break;

		if (!quiet) printf ("read fd of %d, size %d\n", fd, (int)size);
		if (fd != -1) {
			ioctl(fd, PERF_EVENT_IOC_RESET, 0);
			ioctl(fd, PERF_EVENT_IOC_ENABLE,0);

			result=instructions_million();
			ioctl(fd, PERF_EVENT_IOC_DISABLE,0);
			read_result=read(fd,&count,sizeof(long long));

			if (read_result!=sizeof(long long)) {
				fprintf(stderr,"\tImproper return from read: %d\n",read_result);
				test_fail(test_string);
			}

			if (result==CODE_UNIMPLEMENTED) {
				fprintf(stderr,"\tCode unimplemented\n");
				test_fail(test_string);
			}

			if (!quiet) {
				printf("Read %lld instructions\n",count[0]);
			}

			close(fd);
		}
	}
}

void parent(int sock) {

	ssize_t size;
	int fd,i;
	struct perf_event_attr pe;

	memset(&pe,0,sizeof(struct perf_event_attr));
	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.disabled=1;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;

	arch_adjust_domain(&pe,quiet);

	fd=perf_event_open(&pe,0,-1,-1,0);
	if (fd<0) {
		fprintf(stderr,"Error opening leader %llx %s\n",pe.config,strerror(errno));
		test_fail(test_string);
	}

	size = sock_fd_write(sock, "1", 1, fd);
	if (!quiet) printf ("wrote fd %d (size %d)\n", fd, (int)size);

	sleep(1);

	for(i=0;i<20;i++) instructions_million();

}

int main(int argc, char **argv) {

	int sv[2];
	int pid;

	quiet=test_quiet();

	if (!quiet) {
		printf("This test checks passing a perf_event fd over a socket.\n");
	}

	/* Create a pair of connected sockets */
	/* New sockets are in sv[0] and sv[1] */
	if (socketpair(AF_LOCAL, SOCK_STREAM, 0, sv) < 0) {
		fprintf(stderr,"Error creating sockets! %s\n",
			strerror(errno));
		exit(1);
	}

	pid=fork();

	if (pid==0) {
		/* In Child */

		/* Close one side of socket */
		close(sv[0]);
		child(sv[1]);
		exit(0);
	}
	else if (pid==-1) {
		/* error */
		fprintf(stderr,"Error forking! %s\n",
			strerror(errno));
		exit(1);
	}
	else {
		/* In Parent */

		/* Close other side of socket */
		close(sv[1]);
		parent(sv[0]);
	}

	test_pass(test_string);

	return 0;
}
