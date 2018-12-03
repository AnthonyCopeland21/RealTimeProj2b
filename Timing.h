#ifndef TIMING_H
#define TIMING_H

#include <stdlib.h>
#include <stdio.h>
#include <sys/neutrino.h>	// for timers
#include <sys/netmgr.h>		// defines ND_LOCAL_NODE
#include <pthread.h>
#include <stdint.h>       /* for uintptr_t */
#include <hw/inout.h>     /* for in*() and out*() functions */
#include <sys/mman.h>     /* for mmap_device_io() */
#include <termios.h>

#define BASE_ADDRESS (0x280)
#define COUNT_20MS (200)
#define PWM_SHORT_MEDIUM_LONG_CYCLE_RESET (200)

static void set_system_clock_period(void);
static void setup_dio(void);
static timer_t create_pulse_timer(int *ptr_channel_id);
void start_timer(time_t timer_id, int timeOutSec, int timeOutNsec, int periodSec, int periodNsec);
void timer_demo(int channel_id);


#endif //TIMING_H