/*
 * tictoc - poor man's timing utility
 *
 * time program durations. There is a stack of timers so that you can
 * nest the timings.
 *
 * (c) 2005 by Mikio Braun
 */

#ifndef TICTOC_H
#define TICTOC_H

/* number of timers */
#define TICTOC_NUMTIMERS 16

/* start timing. Return 0 if we're out of timers. */
extern int tic();

/* stop timing and return time (in seconds, as double) since tic() */
extern double toc();

#endif
