#ifndef STARTUP_H
#define STARTUP_H

// Struct for each servo
typedef struct {
	unsigned char recipe[50];
	int position;
	int count;
	int wait;
	int loop_location;
	int loop_count;
	int loop_end;
	int running_status;
	int channel_id;
} Servo;

#include "run.h"

#include <stdlib.h>
#include <stdio.h>
#include <sys/neutrino.h>	// for timers
#include <sys/netmgr.h>		// defines ND_LOCAL_NODE
#include <pthread.h>
#include <stdint.h>       /* for uintptr_t */
#include <hw/inout.h>     /* for in*() and out*() functions */
#include <sys/mman.h>     /* for mmap_device_io() */
#include <termios.h>
#include <malloc.h>
#include <math.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>


#define BASE_ADDRESS (0x280)
#define COUNT_20MS (200)
#define PWM_SHORT_MEDIUM_LONG_CYCLE_RESET (200)


int start(void);
static void set_system_clock_period(void);
static void setup_dio(void);
static timer_t create_pulse_timer(int *ptr_channel_id);
void start_timer(time_t timer_id, int timeOutSec, int timeOutNsec, int periodSec, int periodNsec);
void timer_demo(int channel_id);
int setup_timer(void);


#endif  // STARTUP_H