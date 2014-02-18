#!/bin/sh

TRINITY=../../../../trinity/trinity.git/

diff -u fds.c $TRINITY/fds.c
diff -u interesting-numbers.c $TRINITY/interesting-numbers.c
diff -u maps-static.c $TRINITY/maps-static.c
diff -u perf_event_open.c $TRINITY/syscalls/perf_event_open.c
diff -u pids.c $TRINITY/pids.c
diff -u random-address.c $TRINITY/random-address.c
diff -u random.c $TRINITY/random.c
diff -u random-length.c $TRINITY/random-length.c
diff -u random-page.c $TRINITY/random-page.c
diff -u unicode.c $TRINITY/unicode.c
