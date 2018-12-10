#include "run.h"

//GLOBAL VARIABLES
static int done_waiting = 0;
static int received_command = 0;
static uintptr_t d_i_o_port_a_handle;	// digital I/O port A handle
static uintptr_t d_i_o_port_b_handle;	// digital I/O port B handle

// OLD RECIPES
//unsigned char recipe_servo1[] = {MOV, MOV | 5, MOV | 0, MOV | 3, LOOP, MOV | 0, MOV | 1, MOV | 4, END_LOOP, MOV, MOV | 2, MOV, WAIT, MOV | 3, WAIT, MOV | 2, MOV | 3, WAIT | 31, WAIT | 31, WAIT | 31, MOV | 4, RECIPE_END};
//unsigned char recipe_servo1[] = {MOV | 4, SHIFT | 1, LOOP | 4, MOV | 1, WAIT | 1, MOV | 2, SHIFT, WAIT | 2, MOV | 3, END_LOOP, MOV | 5, RECIPE_END};  // Show Tony's grad command
// unsigned char recipe_servo1[] = {MOV | 0, MOV | 1, SHIFT | 1, SHIFT, SHIFT | 1, JUMP | 10, WAIT | 10, WAIT | 5, WAIT | 31, MOV | 4, MOV | 5, RECIPE_END};                                         // Show Josh's grad command
// unsigned char recipe_servo2[] = {MOV | 5, MOV | 3, MOV | 1, MOV | 2, MOV | 5, MOV | 2, MOV | 0, MOV | 5, MOV | 0, RECIPE_END};

//unsigned char left_recipe[] = {MOV, MOV | 5, MOV | 0, MOV | 3, LOOP, MOV | 0, MOV | 1, MOV | 4, END_LOOP, MOV, MOV | 2, MOV, WAIT, MOV | 3, WAIT, MOV | 2, MOV | 3, WAIT | 31, WAIT | 31, WAIT | 31, MOV | 4, RECIPE_END};
unsigned char left_recipe[] = {MOV, MOV | 1, MOV | 2, MOV | 3, MOV | 4, MOV | 5, WAIT | 31, WAIT | 31, WAIT | 31, MOV, RECIPE_END};
unsigned char right_recipe[] = {LOOP | 2, MOV | 0, MOV | 1, MOV | 2, SHIFT | 1, SHIFT, SHIFT | 1, END_LOOP, JUMP | 13, WAIT | 10, WAIT | 5, WAIT | 31, MOV | 4, MOV | 5, RECIPE_END};


// Purpose:		100ms Timing Thread
// Input:		None
// Output: 		None
void *wait_for_done_thread(void *args){
	usleep(200000);
	done_waiting = 1;
	pthread_exit(NULL);
}

// Purpose:		User Interface Thread
// Input:		None
// Output: 		None
void *wait_for_command_thread(void *args){
	char str[5];
	printf(">");
	fgets(str, 5, stdin);
	left.command = str[0];
	right.command = str[1];
	received_command = 1;
	pthread_exit(NULL);
}

// Purpose:		Loop that waits for command and runs recipes
// Input:		None
// Output: 		None
void master_loop(void) {
	pthread_t wait_thread, command_thread;
	pthread_create(&wait_thread, NULL, &wait_for_done_thread, NULL);
	pthread_create(&command_thread, NULL, &wait_for_command_thread, NULL);
	while(1) {
		if(done_waiting == 1){
			// check status, go to next command if running
			if(left.status == 1){
				parse_command(left, left_recipe);
				left.count++;
			}
			if(right.status == 1){
				parse_command(right, right_recipe);
				right.count++;
			}
			//create another waiting thread
			done_waiting = 0;
			pthread_create(&wait_thread, NULL, &wait_for_done_thread, NULL);
		}
		if(received_command == 1){
			// parse command, do the thing
			// printf("Parsing input left: %d %c and right: %d %c\n", left.channel_id, left.command, right.channel_id, right.command);
			parse_input(left);
			parse_input(right);
			left.command = '\0';
			right.command = '\0';
			received_command = 0;
			pthread_create(&command_thread, NULL, &wait_for_command_thread, NULL);
		}
	}
}

// Purpose:		User Interface Parser
// Input:		servo, left or right servo info
// Output: 		None
void parse_input(Servo servo){
	if ((servo.command == 'b' || servo.command == 'B') && servo.status == -1){
		start_recipe(servo);
	}
	else if ((servo.command == 'p' || servo.command == 'P') && servo.status == 1){
		pause_recipe(servo);
	}
	else if ((servo.command == 'c' || servo.command == 'C') && servo.status == 0){
		cont_recipe(servo);
	}
	else if ((servo.command == 'r' || servo.command == 'R') && servo.status <= 0){
		move_right_one(servo);
	}
	else if ((servo.command == 'l' || servo.command == 'L') && servo.status <= 0){
		move_left_one(servo);
	}
}

// Purpose: 	Parse recipe command opcode
// Input:		servo, struct Servo for servo to use
//				recipe, the recipe being used for the servo
// Output:		None
void parse_command(Servo servo, unsigned char *recipe) {
	if ((recipe[servo.count] & JUMP) == JUMP) {
		if ((recipe[servo.count] & 0x1F) > servo.count){
			if (servo.channel_id == left.channel_id)
				left.count = ((recipe[servo.count] & 0x1F)-1);
			else if (servo.channel_id == right.channel_id)
				right.count = ((recipe[servo.count] & 0x1F)-1);
		}

	}
	else if ((recipe[servo.count] & END_LOOP) == END_LOOP){
		end_loop(servo);
	}
	else if ((recipe[servo.count] & SHIFT) == SHIFT) {
		if ((recipe[servo.count] & 0x1F) == 0) {
			move_right_one(servo);
		}
		else if ((recipe[servo.count] & 0x1F) == 1) {
			move_left_one(servo);
		}
	}
	else if ((recipe[servo.count] & MOV) == MOV){
		move_to(servo, (recipe[servo.count]&0x1F));
	}
	else if ((recipe[servo.count] & WAIT) == WAIT){
		wait_time(servo, (recipe[servo.count]&0x1F));
	}
	else if ((recipe[servo.count] & LOOP) == LOOP){
		loop(servo, (recipe[servo.count]&0x1F));
	}
	else if (recipe[servo.count] == 0x00){
		end_recipe(servo);
	}
}

// Purpose:		End of recipe for the servo
// Input:		servo, servo to end recipe
// Output: 		None
void end_recipe(Servo servo) {
	//printf("Ending recipe for servo %d\n", servo.channel_id);
	if (servo.channel_id == left.channel_id){
		left.status = -1;
		left.count = 0;
	}
	else if (servo.channel_id == right.channel_id){
		right.status = -1;
		right.count = 0;
	}
}

// Purpose:		Loop commands set
// Input:		servo, struct Servo of the specific servo being operated
//				loop_count, number of times to go thru loop
// Output:		None
void loop(Servo servo, int loop_count) {
	//printf("Looping with servo %d\n", servo.channel_id);
	if (servo.channel_id == left.channel_id){
		if (left.loop_location != 0){
			left.status = -1;
			left.count = -1;
			left.loop_location = 0;
			left.loop_count = 0;
			left.wait = 0;
			return;
		}
		left.loop_end = loop_count;
		left.loop_location = left.count;
	}
	else if (servo.channel_id == right.channel_id){
		if (right.loop_location != 0){
			right.status = -1;
			right.count = -1;
			right.loop_location = 0;
			right.loop_count = 0;
			right.wait = 0;
			return;
		}
		right.loop_end = loop_count;
		right.loop_location = right.count;
	}
}

// Purpose: 	End of loop, changes location of servo counts
// Input:			left_right, 0 for left servo 1 for right servo
// Output:		None
void end_loop(Servo servo){
	//printf("Ending loop with servo %d\n", servo.channel_id);
	if (servo.channel_id == left.channel_id) {
		if (left.loop_count < left.loop_end) {
			left.count = left.loop_location;
			left.loop_count++;
		}
		else {
			left.loop_count = 0;
			left.loop_location = 0;
		}
	}
	else if (servo.channel_id == right.channel_id){
		if (right.loop_count < right.loop_end) {
			right.count = right.loop_location;
			right.loop_count++;
		}
		else {
			right.loop_count = 0;
			right.loop_location = 0;
		}
	}
}


// Purpose:		Start recipe
// Input:		left_right, 0 for left servo 1 for right servo
// Output:		None
void start_recipe(Servo servo) {
	//printf("Starting servo %d\n", servo.channel_id);
	if (servo.channel_id == left.channel_id) left.status = 1;
	else if (servo.channel_id == right.channel_id) right.status = 1;
}

// Purpose:		Pause specified recipe
// Input:		(int) left_right is an integer where 0 is the left servo and 1 is right servo
// Output: 		None
void pause_recipe(Servo servo) {
	//printf("Pausing servo %d at %d\n", servo.channel_id, servo.count);
	if (servo.channel_id == left.channel_id) left.status = 0;
	else if (servo.channel_id == right.channel_id) right.status = 0;
}

// Purpose:		Continues a specified paused recipe
// Input:		(int) left_right is an integer where 0 is the left servo and 1 is right servo
// Output: 		None
void cont_recipe(Servo servo) {
	//printf("Continuing servo %d at %d\n", servo.channel_id, servo.count);
	if (servo.channel_id == left.channel_id){
		if (!left.status) left.status = 1;
		else printf("Can't continue, recipe is not paused.");
	}
	else if (servo.channel_id == right.channel_id) {
		if (!right.status) right.status = 1;
		else printf("Can't continue, recipe is not paused.");
	}
}

// Purpose: 	Wait given amount of time
// Input:		left_right, 0 for left, 1 for right
//				time, amount of seconds to wait
// Output:		None
void wait_time(Servo servo, int time) {
	//printf("Waiting with servo %d for %f seconds\n", servo.channel_id, time/10.0);
	if (servo.channel_id == left.channel_id){
		if (time >= 0 && time <= 31) {
			if (left.wait < time/2){
				left.wait++;
				left.count--;
			}
			else {
				//printf("Done waiting with servo %d\n", left.channel_id);
				left.wait = 0;
			}
		}
	}
	else if (servo.channel_id == right.channel_id){
		if (time >= 0 && time <= 31) {
			if (right.wait < time/2){
				right.wait++;
				right.count--;
			}
			else {
				//printf("Done waiting with servo %d\n", right.channel_id);
				right.wait = 0;
			}
		}
	}
}

// Purpose:		Moves left or right servo to specified position
// Input:		left_right, 0 for left, 1 for right
//				position, 0 thru 5 position
// Output:		None
void move_to(Servo servo, int position) {
	if (servo.channel_id == left.channel_id){
		if (position <= 5 && position >= 0){
			//printf("Moving servo %d to %d\n", left.channel_id, position);
			left.position = position;
		}
	}
	else if (servo.channel_id == right.channel_id){
		if (position <= 5 && position >= 0){
			//printf("Moving servo %d to %d\n", right.channel_id, position);
			right.position = position;
		}
	}
}

// Purpose:		Moves specified servo 1 value to the left
// Input:		None
// Output: 		None
void move_left_one(Servo servo) {
	if (servo.channel_id == left.channel_id) {
		if (left.position < 5) left.position++;
	}
	else if (servo.channel_id == right.channel_id) {
		if (right.position < 5) right.position++;
	}
}

// Purpose:		Moves specified servo 1 value to the right
// Input:		None
// Output: 		None
void move_right_one(Servo servo) {
	if (servo.channel_id == left.channel_id) {
		if (left.position > 0) left.position--;
	}
	else if (servo.channel_id == right.channel_id) {
		if (right.position > 0) right.position--;
	}
}

// Purpose:		PWM Timer control
// Input:		args, carries channel_id
// Output: 		None
void *timer_running(void *args){
	int channel_id = (int)args;
	uintptr_t port_handle;
	struct _pulse pulse;
	unsigned int counter = 0;
	int position = 0;
	// set port_handle to the proper output pins
	if(channel_id == left.channel_id){
		port_handle = d_i_o_port_a_handle;
		printf("Left channel detected\n");
	}
	else if(channel_id == right.channel_id){
		port_handle = d_i_o_port_b_handle;
		printf("Right channel detected\n");
	}
	while(1) {
		MsgReceivePulse(channel_id, &pulse, sizeof(pulse), NULL);
		if(channel_id == left.channel_id) position = left.position;
		else if(channel_id == right.channel_id) position = right.position;

		if(counter++ > COUNT_20MS) {
			counter = 0;
			out8(port_handle, 0xff);
		}
		else {
			// this will need to change in order to move servo to each 6 positions properly
			if(counter > (position * 3 + 5))
				out8(port_handle, 0);
		}
	}
	pthread_exit(NULL);
}

// Purpose:		Setup of timers, servo, and master loop
// Input:		None
// Output: 		None
int start(void){

	int privity_err;
	privity_err = ThreadCtl(_NTO_TCTL_IO, NULL);
	if (privity_err == -1)
	{
		fprintf(stderr, "can't get root permissions\n");
		return -1 ;
	}

	set_system_clock_period();
	setup_dio();
	setup_servos();

	pthread_t left_thread;
	timer_t left_timer = create_pulse_timer(&left.channel_id);
	pthread_t right_thread;
	timer_t right_timer = create_pulse_timer(&right.channel_id);
	//printf("Channel_left: %d\nChannel_right: %d\n", left.channel_id, right.channel_id);

	start_timer(left_timer, 0, 100000, 0, 100000);
	start_timer(right_timer, 0, 100000, 0, 100000);

	pthread_create(&left_thread, NULL, &timer_running, (void *)left.channel_id);
	pthread_create(&right_thread, NULL, &timer_running, (void *)right.channel_id);

	master_loop();
	return 1;
}


// Purpose:		Setup of servos
// Input:		None
// Output: 		None
void setup_servos(){
	left.command = '\0';
	left.position = 0;
	left.count = 0;
	left.wait = 0;
	left.loop_location = 0;
	left.loop_count = 0;
	left.loop_end = 0;
	left.status = -1;
	left.channel_id = 0;

	right.command = '\0';
	right.position = 0;
	right.count = 0;
	right.wait = 0;
	right.loop_location = 0;
	right.loop_count = 0;
	right.loop_end = 0;
	right.status = -1;
	right.channel_id = 0;
}


// This creates a channel that is returned via the ptr_channel_id parameter.
// We use this channel when we wait for a pulse from the timer.
// Note that this returns the channel ID created by this function.
timer_t create_pulse_timer(int *ptr_channel_id)
{
    timer_t timer_id = 0 ;
 	int pulse_id = 1234 ;	// arbitrary data value to pass with the pulse.
    struct sigevent event;

    *ptr_channel_id = ChannelCreate(0) ;		// Returns to calling program the channel that will get the timer event.

    // from QNX example code in Programmers Guide
    // Set up the timer and timer event.
	// The only part we care about is that the notify is a SIGEV_PULSE and that the coid set to connect the channel to the timer.
	// The rest is ignored.
    event.sigev_notify            = SIGEV_PULSE;
    event.sigev_coid              = ConnectAttach ( ND_LOCAL_NODE, 0, *ptr_channel_id, 0, 0 );
    event.sigev_priority          = getprio(0) ;	// use our current priority.
    event.sigev_code              = 1023;			// arbitrary number to identify this pulse source
    event.sigev_value.sival_int   = pulse_id ;		// arbitrary value to pass with the pulse.

    timer_create(CLOCK_REALTIME, &event, &timer_id) ;	// create but do not start the timer.
    return timer_id ;
}


// Starts a periodic or one time timer depending on the values we use.
void start_timer(time_t timer_id, int timeOutSec, int timeOutNsec, int periodSec, int periodNsec)
{
	struct itimerspec timer_spec;

	timer_spec.it_value.tv_sec = timeOutSec;
	timer_spec.it_value.tv_nsec = timeOutNsec;
	timer_spec.it_interval.tv_sec = periodSec;
	timer_spec.it_interval.tv_nsec = periodNsec;

	timer_settime( timer_id, 0, &timer_spec, NULL );
}

// Sets ALL digital I/O ports to be output ports and get a handle to Digital I/O Port A
void setup_dio(void)
{
	//Digital I/O and D/A Control register at offset 11
	uintptr_t d_i_o_control_handle = mmap_device_io(1, BASE_ADDRESS + 0xb);
	out8(d_i_o_control_handle, 0) ;	// sets the the DIR (direction bits) to outputs.
	d_i_o_port_a_handle = mmap_device_io(1, BASE_ADDRESS + 0x8) ;	// Digital I/O Port A
	d_i_o_port_b_handle = mmap_device_io(1, BASE_ADDRESS + 0x9);	// Digital I/O Port B

}

// This function changes the QNX system clock tick size to just 100 microseconds.
void set_system_clock_period(void)
{
    struct _clockperiod clkper;
    // set clock period to 100 microseconds.
    clkper.nsec       = 10000;		// 100K nanoseconds is 100 microseconds.
    clkper.fract      = 0;
    ClockPeriod (CLOCK_REALTIME, &clkper, NULL, 0);   // set it now.
}
