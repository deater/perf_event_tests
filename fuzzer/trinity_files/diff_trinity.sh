#!/bin/sh

TRINITY=../../../../trinity/trinity.git/

diff -u fds.c $TRINITY/fds.c
diff -u genric-sanitise.c $TRINITY/generic-sanitise.c
diff -u interesting-numbers.c $TRINITY/interesting-numbers.c
diff -u log.c $TRINITY/log.c
diff -u maps-static.c $TRINITY/maps-static.c
diff -u perf_event_open.c $TRINITY/syscalls/perf_event_open.c
diff -u pids.c $TRINITY/pids.c
diff -u random-address.c $TRINITY/random-address.c
diff -u random.c $TRINITY/random.c
diff -u random-length.c $TRINITY/random-length.c
diff -u random-page.c $TRINITY/random-page.c
diff -u shm.c $TRINITY/shm.c
diff -u unicode.c $TRINITY/unicode.c
diff -u utils.c $TRINITY/utils.c


diff -u include/arch.h $TRINITY/include/arch.h
diff -u include/arch-x86-64.h $TRINITY/include/arch-x86-64.h
diff -u include/child.h $TRINITY/include/child.h
diff -u include/compat.h $TRINITY/include/compat.h
diff -u include/constants.h $TRINITY/include/constants.h
diff -u include/exit.h $TRINITY/include/exit.h
diff -u include/log.h $TRINITY/include/log.h
diff -u include/maps.h $TRINITY/include/maps.h
diff -u include/pids.h $TRINITY/include/pids.h
diff -u include/random.h $TRINITY/include/random.h
diff -u include/sanitise.h $TRINITY/include/sanitise.h
diff -u include/shm.h $TRINITY/include/shm.h
diff -u include/syscall.h $TRINITY/include/syscall.h
diff -u include/tables.h $TRINITY/include/tables.h
diff -u include/trinity.h $TRINITY/include/trinity.h
diff -u include/types.h $TRINITY/include/types.h
diff -u include/utils.h $TRINITY/include/utils.h


