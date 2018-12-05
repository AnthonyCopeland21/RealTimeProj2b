#include "run.h"

//OUR SERVO IS GROUP3 SERVO 2015

//unsigned char recipe_servo1[] = {WAIT | 31, WAIT | 31, WAIT | 31};
//unsigned char recipe_servo1[] = {MOV, MOV | 5, MOV | 0, MOV | 3, LOOP, MOV | 0, MOV | 1, MOV | 4, END_LOOP, MOV, MOV | 2, MOV, WAIT, MOV | 3, WAIT, MOV | 2, MOV | 3, WAIT | 31, WAIT | 31, WAIT | 31, MOV | 4, RECIPE_END};
//unsigned char recipe_servo1[] = {MOV | 4, SHIFT | 1, LOOP | 4, MOV | 1, WAIT | 1, MOV | 2, SHIFT, WAIT | 2, MOV | 3, END_LOOP, MOV | 5, RECIPE_END};  // Show Tony's grad command
// unsigned char recipe_servo1[] = {MOV | 0, MOV | 1, SHIFT | 1, SHIFT, SHIFT | 1, JUMP | 10, WAIT | 10, WAIT | 5, WAIT | 31, MOV | 4, MOV | 5, RECIPE_END};                                         // Show Josh's grad command
// unsigned char recipe_servo2[] = {MOV | 5, MOV | 3, MOV | 1, MOV | 2, MOV | 5, MOV | 2, MOV | 0, MOV | 5, MOV | 0, RECIPE_END};


// Purpose:		Loop that waits for command and runs recipes
// Input:		None
// Output: 		None
/*
void master_loop(void) {
	int i = 0;

	// infinite loop for 100ms breaks
	while (1) {
		// wait for 100ms to go by.
		// while waiting for pulse signal, check for a carriage return '\r'
		// parse command after \r found
		// increment found for servo
	}
}

// Purpose: 	Parse command opcode
// Input:			left_right, 0 for left servo 1 for right servo
// 						command, opcode for command
// Output:		None
void parse_command(Servo servo, int command) {
	if ((command & JUMP) == JUMP) {
		if (left_right == 0) {
			if ((command & 0x1F) > left_servo_count){
				left_servo_count = ((command & 0x1F)-1);
			}
		}
		else {
			if ((command & 0x1F) > right_servo_count){
				right_servo_count = ((command & 0x1F)-1);
			}
		}
	}
	else if ((command & END_LOOP) == END_LOOP){
		end_loop(left_right);
	}
	else if ((command & SHIFT) == SHIFT) {
		if ((command & 0x1F) == 0) {
			move_right_one(left_right);
		}
		else if ((command & 0x1F) == 1) {
			move_left_one(left_right);
		}
	}
	else if ((command & MOV) == MOV){
		move_to(left_right, (command&0x1F));
	}
	else if ((command & WAIT) == WAIT){
		wait(left_right, (command&0x1F));
	}
	else if ((command & LOOP) == LOOP){
		loop(left_right, (command&0x1F));
	}
	else if (command == 0x00){
		if (left_right == 0) {
			recipe_status &= ~0xC;
			left_servo_count = 0;
			GPIOE->ODR &= ~GPIO_ODR_ODR_8 ;
		}
		else {
			recipe_status &= ~0x3;
			right_servo_count = 0;
		}
	}
}

// Purpose:		Loop commands set
// Input:			left_right, 0 for left servo 1 for right servo
//						loop_count, number of loops to go through
// Output:		None
void loop(Servo servo) {
	if (left_right == 0) {
		// this condition only occurs when a nested loop occurs
		if (left_servo_loop_location != 0) {
			// error occurs here
			GPIOE->ODR |= GPIO_ODR_ODR_8 ;
			GPIOB->ODR |= GPIO_ODR_ODR_2 ;
			recipe_status &= ~0xC;
			left_servo_count = -1;
			left_servo_loop_location = 0;
			left_servo_loop_count = 0;
			left_servo_wait = 0;
			return;
		}
		left_servo_loop_end = loop_count;
		left_servo_loop_location = left_servo_count;
	}
	else {
		// this condition only occurs when a nested loop occurs
		if (right_servo_loop_location != 0) {
			// error occurs here
			GPIOE->ODR |= GPIO_ODR_ODR_8 ;
			GPIOB->ODR |= GPIO_ODR_ODR_2 ;
			recipe_status &= ~0x3;
			right_servo_count = -1;
			right_servo_loop_location = 0;
			right_servo_loop_count = 0;
			right_servo_wait = 0;
			return;
		}
		right_servo_loop_end = loop_count;
		right_servo_loop_location = right_servo_count;
	}
}

// Purpose: 	End of loop, changes location of servo counts
// Input:			left_right, 0 for left servo 1 for right servo
// Output:		None
void end_loop(Servo servo){
	if (left_right == 0){
		if (left_servo_loop_count < left_servo_loop_end) {
			left_servo_count = left_servo_loop_location;
			left_servo_loop_count++;
		}
		else {
			left_servo_loop_count = 0;
			left_servo_loop_location = 0;
		}
	}
	else {
		if (right_servo_loop_count < right_servo_loop_end) {
			right_servo_count = right_servo_loop_location;
			right_servo_loop_count++;
		}
		else {
			right_servo_loop_count = 0;
			right_servo_loop_location = 0;
		}
	}
}


// Purpose:		Start recipe
// Input:			left_right, 0 for left servo 1 for right servo
// Output:		None
void start_recipe(Servo servo) {
	if (left_right == 0){
		recipe_status |= 0xC;
	}
	else {
		recipe_status |= 0x3;
	}
}

// Purpose:		Pause specified recipe
// Input:			(int) left_right is an integer where 0 is the left servo and 1 is right servo
// Output: 		None
void pause_recipe(Servo servo) {
	if (left_right == 0){
		// set left status to paused
		recipe_status &= 0xB;
	}
	else {
		// set right status to paused
		recipe_status &= 0xE;
	}
}

// Purpose:		Continues a specified paused recipe
// Input:			(int) left_right is an integer where 0 is the left servo and 1 is right servo
// Output: 		None
void cont_recipe(Servo servo) {
	if(left_right == 0) {
		// set left status to running
		recipe_status |= 0xC;
	}
	else {
		// set left status to running
		recipe_status |= 0x3;
	}
}

// Purpose: 	Wait given amount of time
// Input:			left_right, 0 for left, 1 for right
//						time, amount of seconds to wait
// Output:		None
void wait(Servo servo) {
	if(left_right == 0) {
		if (time >= 0 && time <= 31) {
			if (left_servo_wait < (time / 4)){
				left_servo_wait++;
				left_servo_count--;
			}
			else {
				left_servo_wait = 0;
			}
		}
		else {
			// error
		}
	}
	else {
		if (time >= 0 && time <= 31) {
			if (right_servo_wait < (time / 4)){
				right_servo_wait++;
				right_servo_count--;
			}
			else {
				right_servo_wait = 0;
			}
		}
		else {
			// error
		}
	}
}



// Purpose:		Moves left or right servo to specified position
// Input:			left_right, 0 for left, 1 for right
//						position, 0 thru 5 position
// Output:		None
void move_to(Servo servo) {
	if(left_right == 0) {
		if (position <= 5 && position >= 0) {
			TIM2->CCR1 = LEFT_START + (position * SPACING);
			left_servo_position = position;
		}
		else {
			//error
		}
	}
	else {
		if (position <= 5 && position >= 0) {
			TIM2->CCR2 = RIGHT_START + position * SPACING;
			right_servo_position = position;
		}
		else {
			//error
		}
	}
}

// Purpose:		Moves specified servo 1 value to the left
// Input:			None
// Output: 		None
void move_left_one(Servo servo) {
	if (left_right == 0) {
		if (left_servo_position < 5) {
			left_servo_position++;
			TIM2->CCR1 += SPACING;
		}
		else {
			// ERROR: CANNOT MOVE ANY MORE TO THE RIGHT
		}
	}
	else {
		if(right_servo_position < 5) {
			right_servo_position++;
			TIM2->CCR2 += SPACING;
			//change duty cycle to move rigth servo 1 to the right
		}
		else {
			// ERROR: CANNOT MOVE ANY MORE TO THE RIGHT
			// use LEDs for errors
		}
	}
}

// Purpose:		Moves specified servo 1 value to the right
// Input:			None
// Output: 		None
void move_right_one(Servo servo) {
	if (left_right == 0) {
		if (left_servo_position > 0) {
			left_servo_position--;
			TIM2->CCR1 -= SPACING;
		}
		else {
			// ERROR: CANNOT MOVE ANY MORE TO THE LEFT
			// use LEDs for errors
		}
	}
	else {
		if (right_servo_position > 0) {
			right_servo_position--;
			TIM2->CCR2 -= SPACING;
			//change duty cycle to move rigth servo 1 to the left
		}
		else {
			// ERROR: CANNOT MOVE ANY MORE TO THE LEFT
			// use LEDs for errors
		}
	}
}
*/
