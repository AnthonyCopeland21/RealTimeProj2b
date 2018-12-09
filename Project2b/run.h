#ifndef RUN_H
#define RUN_H

#include <stdlib.h>
#include <stdio.h>
#include <sys/neutrino.h>	// for timers
#include <sys/netmgr.h>		// defines ND_LOCAL_NODE
#include <pthread.h>
#include <stdint.h>       // for uintptr_t
#include <hw/inout.h>     // for in*() and out*() functions
#include <sys/mman.h>     // for mmap_device_io()
#include <termios.h>
#include <malloc.h>
#include <math.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

// Struct for each servo
typedef struct {
	char command;
	int position;
	int count;
	int wait;
	int loop_location;
	int loop_count;
	int loop_end;
	int status; // -1 for completed/never started. 0 for stopped. 1 for running
	int channel_id;
} Servo;

static Servo left;
static Servo right;

#define MOV (0x20)						// working
#define RECIPE_END (0x00)				// working
#define WAIT (0x40)						// working
#define LOOP (0x80)						// working
#define END_LOOP (0xA0)					// working
#define SHIFT (0x60)					// Anthony's Command. Shift, 0 to shift right, 1 to shift left
#define JUMP (0xC0)

/*
#define SPACING (300)
#define LEFT_START (550)    //500
#define RIGHT_START (550)   //750
*/

#define BASE_ADDRESS (0x280)
#define COUNT_20MS (200)
#define PWM_SHORT_MEDIUM_LONG_CYCLE_RESET (200)


void master_loop(void);

void *wait_for_done_thread(void *args);
void *wait_for_command_thread(void *args);
void parse_input(Servo servo);
void parse_command(Servo servo, unsigned char *recipe);
void pause_recipe(Servo servo);
void cont_recipe(Servo servo);
void move_to(Servo servo, int position);
void wait_time(Servo servo, int time);
void move_right_one(Servo servo);
void move_left_one(Servo servo);
void start_recipe(Servo servo);
void loop(Servo servo, int loop_count);
void end_loop(Servo servo);
void end_recipe(Servo servo);

void *timer_running(void *args);
void setup_servos();
int start(void);
void set_system_clock_period(void);
void setup_dio(void);
timer_t create_pulse_timer(int *ptr_channel_id);
void start_timer(time_t timer_id, int timeOutSec, int timeOutNsec, int periodSec, int periodNsec);


#endif //  RUN_H
