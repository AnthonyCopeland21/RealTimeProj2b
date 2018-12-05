#include "startup.h"

static uintptr_t d_i_o_port_a_handle ;	// digital I/O port handle
static Servo left;
static Servo right;

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

	left.channel_id = 0;
	timer_t timer_id = (timer_t) 0;
	right.channel_id = 0;

	timer_id = create_pulse_timer(&left.channel_id);
	struct _pulse pulse;

	int *timer_value;
	timer_value = 40 * 100000;
	// this will start the timer with said duty cycle
	// this should really happen in master_loop
	start_timer(timer_id, 0, &timer_value, 0, &timer_value);
	// infinite loop for debugging
	while(1){
		// when received, the interrupt has occurred
		//out8(d_i_o_port_a_handle, 0xff);
		//MsgReceivePulse(left.channel_id, &pulse, sizeof(pulse), NULL);
		//out8(d_i_o_port_a_handle, 0);
		//MsgReceivePulse(left.channel_id, &pulse, sizeof(pulse), NULL);

		// when received, the interrupt has occurred
		/*start_timer(timer_id, 0, 100000 * 100, 0, 100000 * 100);
		out8(d_i_o_port_a_handle, 0xff);
		MsgReceivePulse(left.channel_id, &pulse, sizeof(pulse), NULL);
		out8(d_i_o_port_a_handle, 0);
		MsgReceivePulse(left.channel_id, &pulse, sizeof(pulse), NULL);*/
	}
}

// Loop forever getting a pulse based on what we set for the timer.
// This demonstrates a crude form of PWM output.
void timer_demo(int channel_id)
{
	struct _pulse pulse ;
	static unsigned int pwm_pulse_counter = 0 ;
	static unsigned int counter = 0 ;

	while ( 1 )
	{
		// The idea here is that we set up the timer to fire once every 100 microseconds (the fastest
		// we can go due to the system clock resolution). We then control the counting of the number
		// of pulses that have occurred (count the number of cycles on and the number off)
		MsgReceivePulse ( channel_id, &pulse, sizeof( pulse ), NULL );	// block until that timer fires

		if ( counter++ > COUNT_20MS )	// wait for 20 ms for the end of the PWM period.
		{
			counter = 0 ;	// restart our 20 ms counter

			// When we have sent 200 PWM pulses reset the pulse width back to the short width pulse (approx 500 microseconds)
			if ( pwm_pulse_counter++ > PWM_SHORT_MEDIUM_LONG_CYCLE_RESET )	// count the number of pulses since last reset -- reset every 4 seconds
				pwm_pulse_counter = 0 ;

			out8( d_i_o_port_a_handle, 0xff ) ;		// At the end of the 20 ms. period start the next PWM pulse
													// by setting all of the A digital output lines to 1.
		}
		else
		{
			// Demonstrate an arbitrary set of three different pulse widths.
			// Our servos can handle pulse widths from about 400 microseconds through 2000 microseconds
			// for the full range of positions. If you set the pulse width to 4 you get 400 microseconds.
			// This code cycles between 500 microsecond, 1000 microsecond, and 1500 microsecond pulse widths.
			int pulse_width_in_microseconds = 5 ;		// set to 500 microsecond pulse.

			// When less than 50 we are sending a 500 microsecond pulse. (This is the 1st second of the 4 second repeat cycle.)
			// When between 50 and 100 we are sending a 1000 microsecond pulse. (This is the 2nd second of 4 second repeat cycle.)
			// When over 100 we are sending a 1500 microsecond pulse. (This is the last two seconds of the 4 second repeat cycle.)
			if ( pwm_pulse_counter > 50 && pwm_pulse_counter < 100 )	// are we between 1 second and 2 seconds?
				pulse_width_in_microseconds = 10 ;						// set pulse width to 1000 microseconds (10 * 100 microseconds per loop)
			else if ( pwm_pulse_counter >= 100 )						// are we after 2 seconds?
				pulse_width_in_microseconds = 15 ;						// set pulse width to 1500 microseconds

			// When we reach the end of the current pulse width set the output back to low for the
			// rest of the 20 ms. period.
			if ( counter > pulse_width_in_microseconds )	// set output high for the duty cycle.
				out8( d_i_o_port_a_handle, 0 ) ;			// Sets all of the A digital output lines to 0
		}
	}
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
void start_timer(time_t timer_id, unsigned int *timeOutSec, unsigned int *timeOutNsec, unsigned int *periodSec, unsigned int *periodNsec)
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
	uintptr_t d_i_o_control_handle = mmap_device_io(1, BASE_ADDRESS + 0xb) ;
	out8(d_i_o_control_handle, 0) ;	// sets the the DIR (direction bits) to outputs.
	d_i_o_port_a_handle = mmap_device_io(1, BASE_ADDRESS + 0x8) ;	// Digital I/O Port A
}

// This function changes the QNX system clock tick size to just 100 microseconds.
void set_system_clock_period(void)
{
    struct _clockperiod clkper;
    // set clock period to 100 microseconds.
    clkper.nsec       = 100000;		// 100K nanoseconds is 100 microseconds.
    clkper.fract      = 0;
    ClockPeriod (CLOCK_REALTIME, &clkper, NULL, 0);   // set it now.
}
