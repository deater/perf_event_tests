#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/mman.h>

#include <signal.h>
#include <setjmp.h>

#include "perf_event.h"
#include "perf_helpers.h"
#include "perf_barrier.h"

#include "perf_fuzzer.h"
#include "fuzzer_determinism.h"
#include "fuzzer_logging.h"
#include "fuzzer_stats.h"

#include "perf_attr_print.h"

#include "fuzz_mmap.h"

#define MAX_MMAPS	1024

struct our_mmap_t {
	int active;
	char *addr;
	int size;
	int fd;
	int prot;
	int flags;
	long long head,aux_head;
	int aux;
	int parent_mmap;
	int which_event;
} mmaps[MAX_MMAPS];

static int active_mmaps=0;

static int mmap_random_prot(void) {

	int prot=PROT_NONE;

	/* Usually return valid expected perf_event values */
	if (rand()%10!=0) {
		return PROT_READ|PROT_WRITE;
	}

	if (rand()%2) prot|=PROT_EXEC;
	if (rand()%2) prot|=PROT_READ;
	if (rand()%2) prot|=PROT_WRITE;

	return prot;
}

static int mmap_random_size(int notaux) {

	int size;

	/* to be valid we really want to be 2^x pages */
	switch(rand()%3) {
		case 0: size=(rand()%64)*getpagesize();
			break;
			/* Non aux should be 1+2^x */
		case 1: size=(notaux+(1<<rand()%10) )*getpagesize();
			break;
		default: size=rand()%65535;
	}

	return size;
}

static int mmap_random_flags(void) {

	int flags;

	/* Usually return valid expected perf_event values */
	if (rand()%10!=0) {
		return MAP_SHARED;
	}

	if (rand()%2) {
		flags=MAP_SHARED;
	}
	else {
		flags=MAP_PRIVATE;
	}

	/* ANONYMOUS just ignores fd and so buffers created	*/
	/* are not really perf_event mmap buffers.		*/
	/* if (rand()%2) flags|=MAP_ANONYMOUS;			*/

/* MAP_32BIT only exists on x86_64 */
#ifdef MAP_32BIT
	if (rand()%2) flags|=MAP_32BIT;
#endif
	if (rand()%2) flags|=MAP_DENYWRITE;
	if (rand()%2) flags|=MAP_EXECUTABLE;
	if (rand()%2) flags|=MAP_FILE;
	if (rand()%2) flags|=MAP_FIXED;
	if (rand()%2) flags|=MAP_GROWSDOWN;
#ifdef MAP_HUGETLB
	if (rand()%2) flags|=MAP_HUGETLB;
#endif
	if (rand()%2) flags|=MAP_LOCKED;
	if (rand()%2) flags|=MAP_NONBLOCK;
	if (rand()%2) flags|=MAP_NORESERVE;
	if (rand()%2) flags|=MAP_POPULATE;
#ifdef MAP_STACK
	if (rand()%2) flags|=MAP_STACK;
#endif

	/* Not supported on normal kernels */
	/*	if (rand()%2) flags|=MAP_UNINITIALIZED; */

	return flags;

}



static int find_empty_mmap(void) {

	int i;

	for(i=0;i<MAX_MMAPS;i++) {
		if (!mmaps[i].active) {
			return i;
		}
	}
	return -1;

}

static int find_random_active_mmap(void) {

        int i,x,j=0;

        if (active_mmaps<1) {
//		printf("No active mmaps %d\n",active_mmaps);
		return -1;
	}

        x=rand()%active_mmaps;

        for(i=0;i<MAX_MMAPS;i++) {
                if (mmaps[i].active) {
                        if (j==x) {
//				printf("J=%d X=%d I=%d %p\n",j,x,i,mmaps[i].addr);
				return i;
			}
                        j++;
                }
        }

	printf("Fail random mmaps\n");
        return -1;
}

long long perf_mmap_aux_read(int which) {

	struct perf_event_mmap_page *control_page;
	long long head;
	int parent_mmap;

	parent_mmap=mmaps[which].parent_mmap;

	stats.mmap_aux_read_attempts++;

	if (!(mmaps[which].prot&PROT_READ)) return 0;

	if (!(mmaps[parent_mmap].prot&PROT_READ)) return 0;

	if (!mmaps[parent_mmap].active) return 0;

	if (mmaps[parent_mmap].addr==MAP_FAILED) return 0;

	if (mmaps[parent_mmap].size==0) return 0;

	control_page=(struct perf_event_mmap_page *)mmaps[parent_mmap].addr;

	if (control_page==NULL) {
		return -1;
	}
	head=control_page->aux_head;
	rmb(); /* Must always follow read of data_head */

	/* Mark all as read */
	if (mmaps[parent_mmap].prot&PROT_WRITE) {
		control_page->aux_tail=head;
		mmaps[which].aux_head=head;
	}

	stats.mmap_aux_read_successful++;

	return head;

}

long long perf_mmap_read(int which) {

	struct perf_event_mmap_page *control_page;

	long long head;

	if (mmaps[which].aux) {
		return perf_mmap_aux_read(which);
	}

	stats.mmap_read_attempts++;

	if (!(mmaps[which].prot&PROT_READ)) return 0;

	control_page=(struct perf_event_mmap_page *)mmaps[which].addr;

	if (!mmaps[which].active) return 0;

	if (mmaps[which].addr==MAP_FAILED) return 0;

	if (mmaps[which].size==0) return 0;

	if (control_page==NULL) {
		return -1;
	}
	head=control_page->data_head;
	rmb(); /* Must always follow read of data_head */

	/* Mark all as read */
	if (mmaps[which].prot&PROT_WRITE) {
		control_page->data_tail=head;
		mmaps[which].head=head;
	}

	stats.mmap_read_successful++;

	return head;
}

static sigjmp_buf j_buf;

/* Called if we get a segfault */
/* We only set this up when trying to trash a mmap buffer */
static void sigsegv_handler(int ignored) {

	/* restore the state to where we called sigsetjmp */
	siglongjmp(j_buf, 1);
}



/* The first mmap() page is writeable so you can set the tail pointer */
/* So try over-writing it to see what happens.                        */

void trash_random_mmap(void) {

	int i,value;

	stats.mmap_trash_attempts++;

	i=find_random_active_mmap();

	/* Exit if no events */
	if (i<0) return;

//	printf("Found %d %p\n",i,mmaps[i].addr);

	value=rand();

	if (ignore_but_dont_skip.trash_mmap) return;

	/* can't write high pages? */
	//event_data[i].mmap_size);

//	*(event_data[i].mmap)=0xff;

//	printf("Attempting to trash %p: ",mmaps[i].addr);
//	mmap_print_prot(mmaps[i].prot);
//	mmap_print_flags(mmaps[i].flags);
//	printf("\n");

	/* If not writable then we segfault */
	if (!(mmaps[i].prot & PROT_WRITE)) {
//		printf("trash: Not writable, skipping\n");
		return;
	}

	if ( (mmaps[i].aux) && (mmaps[i].prot&PROT_WRITE)) {
//		printf("trash: writable aux, skipping\n");
		return;
	}

	/* Before we attempt to write the mmap, setup a signal handler */
	signal(SIGSEGV, sigsegv_handler);

	/* Save state before the memset */
	/* If the memset segfaults then we will skip it */
	if (!sigsetjmp(j_buf, 1)) {
		memset(mmaps[i].addr,value, 1);//getpagesize());
	}
	else {
		printf("Memset of mmap#%d at %p caused segfault!\n",i,mmaps[i].addr);
		perf_pretty_print_mmap_flags(mmaps[i].flags);
		perf_pretty_print_mmap_prot(mmaps[i].prot);
		printf("size %d fd %d\n",mmaps[i].size,mmaps[i].fd);
		printf("event: cpu=%d pid=%d group_fd=%d flags=%ld ",
			event_data[mmaps[i].which_event].cpu,
			event_data[mmaps[i].which_event].pid,
			event_data[mmaps[i].which_event].group_fd,
			event_data[mmaps[i].which_event].flags
			);
		perf_pretty_print_attr(stdout,
			&event_data[mmaps[i].which_event].attr,
			mmaps[i].fd);
	}

	/* Disable the seigsegv handler */
	signal(SIGSEGV, SIG_DFL);

	if (logging&TYPE_TRASH_MMAP) {
		sprintf(log_buffer,"Q %d %d %d\n",
			value,
			getpagesize(),
			mmaps[i].fd);
		write(log_fd,log_buffer,strlen(log_buffer));
	}

	stats.mmap_trash_successful++;

//	printf("Done trashing\n");
}


/* NOTE!  This function is not re-entrant!			*/
/* If we ever want it to be we are going to have to add locking	*/
/* We already had problems where signal handlers interrupt after*/
/* active is set to 1 but addr is not valid.			*/
int setup_mmap(int which) {

	int i,prot,flags,size;

	i=find_empty_mmap();
	if (i<0) return -1;

	size=mmap_random_size(1);
	prot=mmap_random_prot();
	flags=mmap_random_flags();

	/* need locking? */
//	mmaps[i].active=1;
	mmaps[i].addr=MAP_FAILED;
	mmaps[i].prot=prot;
	mmaps[i].flags=flags;
	mmaps[i].size=size;
	mmaps[i].aux=0;

	if (!ignore_but_dont_skip.mmap) {

		stats.mmap_attempts++;
		stats.total_syscalls++;
		mmaps[i].addr=mmap(NULL, size,
			prot, flags,
			event_data[which].fd, 0);

		/* If running as root, with MAP_FIXED, NULL is a valid addr */
//		if (mmaps[i].addr==NULL) {
//			printf("This can't happen???\n");
//		}

		if (mmaps[i].addr==MAP_FAILED) {
			mmaps[i].addr=MAP_FAILED;
#if LOG_FAILURES
			if (logging&TYPE_MMAP) {
				if (trigger_failure_logging) {
					sprintf(log_buffer,"# M %d %d %p\n",
						mmaps[i].size,
						event_data[which].fd,
						mmaps[i].addr);
					write(log_fd,log_buffer,
						strlen(log_buffer));
				}
			}
#endif
			return -1;
		}

		else {
			active_mmaps++;
			event_data[which].mmap=i;
			mmaps[i].fd=event_data[which].fd;
			mmaps[i].which_event=which;

			if (logging&TYPE_MMAP) {
				sprintf(log_buffer,"M %d %d %p\n",
					mmaps[i].size,
					event_data[which].fd,
					mmaps[i].addr);
				write(log_fd,log_buffer,strlen(log_buffer));
			}

			stats.mmap_successful++;
		}
	}

	mmaps[i].active=1;
//	if (mmaps[i].addr==NULL) printf("%d.active=1 %p\n",i,mmaps[i].addr);

	/* Randomly try to set up an aux mmap too */
	if (rand()%4==0) {
		setup_mmap_aux(which,i);
	}



	return 0;
}

int setup_mmap_aux(int which_fd, int which_mmap) {

	int i,flags,prot,size;

	i=find_empty_mmap();
	if (i<0) return -1;

	size=mmap_random_size(0);
	prot=mmap_random_prot();
	flags=mmap_random_flags();

	/* need locking? */
	mmaps[i].addr=MAP_FAILED;

	mmaps[i].prot=prot;
	mmaps[i].flags=flags;
	mmaps[i].size=size;
	mmaps[i].aux=1;
	mmaps[i].parent_mmap=which_mmap;

	if (!ignore_but_dont_skip.mmap) {

		stats.mmap_aux_attempts++;

		/* Hook into existing mmap */
		struct perf_event_mmap_page *mmap_control;

		mmap_control = (struct perf_event_mmap_page *)
				mmaps[which_mmap].addr;

		if ((mmap_control!=NULL) &&
			(mmaps[which_mmap].prot&PROT_WRITE)) {
			mmap_control->aux_offset=mmaps[which_mmap].size;
			mmap_control->aux_size=mmaps[i].size;
		}

		mmaps[i].addr=mmap(NULL, size,
			prot, flags,
			event_data[which_fd].fd, mmaps[which_mmap].size);

		if (mmaps[i].addr==MAP_FAILED) {
			mmaps[i].addr=MAP_FAILED;
#if LOG_FAILURES
			if (logging&TYPE_MMAP) {
				if (trigger_failure_logging) {
					sprintf(log_buffer,"# M %d %d %p\n",
						mmaps[i].size,
						event_data[which_fd].fd,
						mmaps[i].addr);
					write(log_fd,log_buffer,
						strlen(log_buffer));
				}
			}
#endif
			return -1;
		}

		else {
			active_mmaps++;
			event_data[which_fd].mmap=i;
			mmaps[i].fd=event_data[which_fd].fd;

			if (logging&TYPE_MMAP) {
				sprintf(log_buffer,"M %d %d %p\n",
					mmaps[i].size,
					event_data[which_fd].fd,
					mmaps[i].addr);
				write(log_fd,log_buffer,strlen(log_buffer));
			}

			stats.mmap_aux_successful++;
		}
	}

	mmaps[i].active=1;
//	if (mmaps[i].addr==NULL) printf("b%d.active=1 %p\n",i,mmaps[i].addr);

	return 0;
}

void unmap_mmap(int i,int from_sigio) {

	int result;

	stats.mmap_unmap_attempts++;
	stats.total_syscalls++;

	if (i<0) {
		fprintf(stderr,"ERROR! Invalid mmap index %d\n",i);
	}

	if (mmaps[i].addr==MAP_FAILED) return;
//	printf("Unmapping %p\n",mmaps[i].addr);

	/* moved up to avoid a race with signal/read_mmap */
	mmaps[i].active=0;
//	if (mmaps[i].addr==NULL) printf("%d.active=0 sigio=%d (%p)\n",
//		i,from_sigio,mmaps[i].addr);

	result=munmap(mmaps[i].addr,mmaps[i].size);
	if ((!from_sigio) && (logging&TYPE_MMAP)) {
		sprintf(log_buffer,"U %d %d %p\n",
			mmaps[i].fd, //event_data[i].fd,
			mmaps[i].size,
			mmaps[i].addr);
		write(log_fd,log_buffer,strlen(log_buffer));
	}
	mmaps[i].addr=MAP_FAILED;

	if (result==0) {
		stats.mmap_unmap_successful++;
		active_mmaps--;
	}
}


void mmap_random_event(int type) {

	int which;

	switch(rand()%6) {

		case 0:	/* mmap random */
			which=find_random_active_event();
			setup_mmap(which);
			break;
		case 1: /* aux random */
			break;
		case 2: /* munmap random */
			which=find_random_active_mmap();
			if (which>=0) unmap_mmap(which,0);
			break;
		case 3: /* mmap read */
			which=find_random_active_mmap();
			if (which>=0) perf_mmap_read(which);
			break;
		case 4: /* trash mmap */
			if (type & TYPE_TRASH_MMAP) {
				trash_random_mmap();
			}
			break;

		default:
			break;
	}

	return;
}
