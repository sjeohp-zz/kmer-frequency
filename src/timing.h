// #include "timing.h"

#include <sys/time.h>
// #include <ctime>
#include <limits.h>
#include <stdio.h>

unsigned long long start_ = 0;
unsigned long long duration_ = ULLONG_MAX;	

unsigned long long timestamp_usec()
{
  struct timeval now;
  gettimeofday(&now, NULL);
  return  now.tv_usec + (unsigned long long)now.tv_sec * 1000000L;
}

void time_start() {
	start_ = timestamp_usec();
	duration_ = ULLONG_MAX;
}

void time_stop() {
	unsigned long long stop = timestamp_usec();
	duration_ = stop - start_;
	start_ = 0;
	if (duration_ == stop) {
		printf("TIMING WARNING: it looks like you didn't call timing_start() before timing_stop().\n");
	}
}

unsigned long long time_elapsed() {
	return duration_;
}