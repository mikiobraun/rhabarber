/*
 * Warning, this code assumes that tv.tv_sec is a int variable
 */

#include "tictoc.h"

#include <time.h>
#include <sys/time.h>

static int tic_saved_sec[TICTOC_NUMTIMERS];
static double tic_saved_time[TICTOC_NUMTIMERS];
static int tic_tos = 0;

int tic()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
    
  if(tic_tos == TICTOC_NUMTIMERS)
    return 0;
  else {
    tic_saved_sec[tic_tos] = tv.tv_sec;
    tic_saved_time[tic_tos] = tv.tv_usec * 1e-6;
    tic_tos++;
    return 1;
  }
}

double toc()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);

  if(tic_tos == 0)
    return 0;
  else {
    tic_tos--;
    tv.tv_sec -= tic_saved_sec[tic_tos];
    double now = tv.tv_sec + tv.tv_usec*1e-6;

    return now - tic_saved_time[tic_tos];
  }
}
