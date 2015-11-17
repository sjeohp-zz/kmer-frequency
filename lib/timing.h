#ifndef TIMING_H
#define TIMING_H

namespace timing {

unsigned long long timestamp_usec();
void start();
void stop();
unsigned long long elapsed_usec_running();
unsigned long long elapsed_usec_stopped();
double elapsed_ms_stopped();
void print_micro();
void print_milli();
void print_sec();

}

#endif