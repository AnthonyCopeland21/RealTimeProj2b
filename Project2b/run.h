#ifndef RUN_H
#define RUN_H

#include "startup.h"

#define MOV (0x20)						// working
#define RECIPE_END (0x00)			// working
#define WAIT (0x40)						// working
#define LOOP (0x80)						// working
#define END_LOOP (0xA0)				// working
#define SHIFT (0x60)					// Anthony's Command. Shift, 0..1 0110 0000
															// 0 to shift right, 1 to shift left
#define JUMP (0xC0)

#define SPACING (300)
#define LEFT_START (550)    //500
#define RIGHT_START (550)   //750

void master_loop(void);

// each of these will change the register
/*
void parse_command(int left_right, int command);
void pause_recipe(int left_right);
void cont_recipe(int left_right);
void move_to(int left_right, int position);
void wait(int left_right, int time);
void move_right_one(int left_right);
void move_left_one(int left_right);
void start_recipe(int left_right);
void loop(int left_right, int loop_count);
void end_loop(int left_right);
*/

#endif //  RUN_H
