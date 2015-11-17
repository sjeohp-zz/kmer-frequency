#include "timing.h"

#include <sys/time.h>
#include <ctime>
#include <limits.h>
#include <stdio.h>

namespace timing {
	
typedef unsigned long long ull;

ull start_ = 0;
ull duration_ = ULLONG_MAX;	

unsigned long long timestamp_usec()
{
  struct timeval now;
  gettimeofday(&now, NULL);
  return  now.tv_usec + (ull)now.tv_sec * 1000000L;
}

void start() {
	start_ = timestamp_usec();
	duration_ = ULLONG_MAX;
}

void stop() {
	ull stop = timestamp_usec();
	duration_ = stop - start_;
	start_ = 0;
	if (duration_ == stop) {
		printf("TIMING WARNING: it looks like you didn't call start() before stop().\n");
	}
}

unsigned long long elapsed_usec_running() {
	return timestamp_usec() - start_;
}

unsigned long long elapsed_usec_stopped() {
	return duration_;
}

double elapsed_ms_stopped() {
	return duration_ / 1000.0L;
}

void print_micro() {
	if (duration_ == ULLONG_MAX) {
		printf("TIMING WARNING: it looks like you didn't called stop() before printing.\n");
	}
	printf("Elapsed time: %lluus\n", (unsigned long long)duration_);
}

void print_milli() {
	if (duration_ == ULLONG_MAX) {
		printf("TIMING WARNING: it looks like you didn't called stop() before printing.\n");
	}
	printf("Elapsed time: %Lfms\n", duration_ / 1000.0L);
}

void print_sec() {
	if (duration_ == ULLONG_MAX) {
		printf("TIMING WARNING: it looks like you didn't called stop() before printing.\n");
	}
	printf("Elapsed time: %Lfs\n", duration_ / 1000000.0L);
}

}