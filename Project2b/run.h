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

// STRUCTS
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

// GLOBAL VARIABLES
static Servo left;
static Servo right;

// CONSTANTS
#define MOV (0x20)
#define RECIPE_END (0x00)
#define WAIT (0x40)
#define LOOP (0x80)
#define END_LOOP (0xA0)
#define SHIFT (0x60)
#define JUMP (0xC0)

#define BASE_ADDRESS (0x280)
#define COUNT_20MS (200)
#define PWM_SHORT_MEDIUM_LONG_CYCLE_RESET (200)


// Main Loop
void master_loop(void);

// Threads
void *timer_running(void *args);
void *wait_for_done_thread(void *args);
void *wait_for_command_thread(void *args);

// User Interface and servo control
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

// Starting up timers and servos
void setup_servos();
int start(void);
void set_system_clock_period(void);
void setup_dio(void);
timer_t create_pulse_timer(int *ptr_channel_id);
void start_timer(time_t timer_id, int timeOutSec, int timeOutNsec, int periodSec, int periodNsec);


#endif //  RUN_H
