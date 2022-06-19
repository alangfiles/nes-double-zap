// todo:
// [] change angle of the ball (y and x probably) whenever it's hit close to the goal line
// [] flash two frames (top and bottom) of the ball to put spin on it?
// [] add title screen with button holding down

#include "LIB/neslib.h"
#include "LIB/nesdoug.h"
#include "LIB/zaplib.h"
#include "doublezap.h"
#include "sprites.h"

void main(void)
{

	ppu_off(); // screen off

	pal_bg(palette_bg);	 //	load the palette
	pal_spr(palette_sp); //	load the palette
	set_vram_buffer();	 // do at least once, sets a pointer to a buffer

	bank_spr(1); // sprites use the 2nd tileset

	ppu_wait_nmi(); // wait

	game_mode = MODE_TITLE;

	//	music_play(0); // silence

	set_vram_buffer(); // points ppu update to vram_buffer, do this at least once
	ppu_on_all();			 // turn on screen
	clear_balls_and_hits();

	while (1)
	{

		while (game_mode == MODE_TITLE) // gameloop
		{
			ppu_wait_nmi(); // wait till beginning of the frame
			// todo, title stuff
			draw_bg();

			game_mode = MODE_GAME;
		}
		while (game_mode == MODE_GAME) // gameloop
		{
			ppu_wait_nmi(); // wait till beginning of the frame

			zap1_hit_detected = 0;
			zap2_hit_detected = 0;

			oam_clear();
			read_input_triggers();
			draw_score();
			update_cooldown();
			draw_cooldown();

			// should check if ANY ball is on the screen or if we need a new one
			number_of_balls_active = 0;
			for (index = 0; index < MAX_BALLS; ++index)
			{
				if (balls_type[index] != TURN_OFF)
				{
					++number_of_balls_active;
				}
			}

			if (number_of_balls_active != 0)
			{
				move_balls();
				draw_balls();
				if (trigger1_pulled || trigger2_pulled)
				{
					trigger_pulled();
				}
			}
			else
			{
				new_ball();
			}

			gray_line();
		}
	}
}

void trigger_pulled(void)
{
	/*
	 * Goes through the required actions for a trigger pull
	 * blank the screen
	 * for each object,
	 *   for each frame
	 *     draw boxes
	 *     detect hits
	 */

	// trigger pulled, play bang sound
	// sfx_play(0, 0);

	// blank the screen
	oam_clear();
	ppu_mask(0x16); // BG off, won't happen till NEXT frame
	ppu_wait_nmi(); // wait for that blank frame

	index = 0;
	number_of_balls_active = 0;
	for (index = 0; index < MAX_BALLS; ++index)
	{
		if (balls_type[index] != TURN_OFF)
		{
			++number_of_balls_active;
		}
	}
	for (index = 0; index < MAX_BALLS; ++index)
	{
		if (balls_type[index] == TURN_OFF)
			continue; // we found an empty spot

		oam_clear(); // clear the NEXT frame
		draw_box();	 // draw a ball on the next frame
		ppu_wait_nmi();
		read_zapper_hits();
		if (zap1_hit_detected == 1 || zap2_hit_detected == 1)
		{
			handle_ball_hit();
			break;
		}
	}

	ppu_mask(0x1e); // bg on, won't happen till NEXT frame
}

void update_cooldown(void)
{
	if (zap1_cooldown > 0)
	{
		--zap1_cooldown;
	}
	if (zap2_cooldown > 0)
	{
		--zap2_cooldown;
	}
}

void handle_ball_hit(void)
{
	if (zap1_hit_detected == 1)
	{
		balls_x_direction[index] = GOING_RIGHT;
	}
	if (zap2_hit_detected == 1)
	{
		balls_x_direction[index] = GOING_LEFT;
	}

	if (zap1_hit_detected == 1 || zap2_hit_detected == 1) // if it's hit update the speed
	{
		balls_x_speed[index] += DEFAULT_SPEED_STEP;
		if (get_frame_count() & 0x01 == 1)
		{
			balls_y_speed[index] -= DEFAULT_SPEED_STEP;
		}
		else
		{
			balls_y_speed[index] += DEFAULT_SPEED_STEP;
		}

		// handle hit / split up ball (for non-small balls)
		balls_hits[index] = balls_hits[index] + 1;
		if (balls_hits[index] > MAX_HITS && balls_type[index] != SMALL_BALL)
		{
			// destroy this ball, and spawn two less ones

			// find an empty spot in the array
			for (index2 = 0; index2 < MAX_BALLS; ++index2)
			{
				if (balls_type[index2] == TURN_OFF)
				{
					break;
				}
			}

			switch (balls_type[index])
			{
			case MEDIUM_BALL:
				balls_type[index2] = SMALL_BALL;
				break;
			case LARGE_BALL:
				balls_type[index2] = MEDIUM_BALL;
			default:
				break;
			}
			// copy over the old parent's values
			// temp1 = balls_x[index];
			balls_x[index2] = balls_x[index];
			balls_y[index2] = balls_y[index];

			balls_x_speed[index2] = balls_x_speed[index];
			balls_y_speed[index2] = balls_y_speed[index];
			balls_x_direction[index2] = balls_x_direction[index];
			balls_y_direction[index2] = GOING_UP;

			// now that it's copied, overwrite the current one
			balls_type[index] = balls_type[index2];
			balls_y_direction[index] = GOING_DOWN;

			if (balls_y_speed[index] == 0)
			{
				balls_y_speed[index] += DEFAULT_SPEED_STEP;
			}
		}
	}
}

void read_input_triggers(void)
{
	trigger1_pulled = 0;
	trigger2_pulled = 0;

	zap1_ready = pad1_zapper ^ 1; // XOR last frame, make sure not held down still
	zap2_ready = pad2_zapper ^ 1; // XOR last frame, make sure not held down still

	// is trigger pulled?
	pad1_zapper = zap_shoot(0); // controller slot 1
	pad2_zapper = zap_shoot(1); // controller slot 2

	if ((pad1_zapper == 1) && (zap1_ready) && zap1_cooldown == 0)
	{
		trigger1_pulled = 1;
		zap1_cooldown = MAX_COOLDOWN;
	}
	if ((pad2_zapper == 1) && (zap2_ready) && zap2_cooldown == 0)
	{
		trigger2_pulled = 1;
		zap2_cooldown = MAX_COOLDOWN;
	}

	// debug code (for using controller to shoot and miss)
	//  pad1 = pad_poll(0);
	//  pad1_new = get_pad_new(0);

	// if ((pad1_new & PAD_A) && zap1_cooldown == 0) //((pad1_zapper) && (zap1_ready));
	// {
	// 	trigger1_pulled = 1;
	// 	zap1_cooldown = MAX_COOLDOWN;
	// }
	// if ((pad1_new & PAD_B) && zap2_cooldown == 0) //((pad2_zapper) && (zap2_ready));
	// {
	// 	trigger2_pulled = 1;
	// 	zap2_cooldown = MAX_COOLDOWN;
	// }
}

void read_zapper_hits(void)
{
	// only reads the hit for the zapper pulled
	// this should be the read code:
	if (trigger1_pulled == 1)
	{
		zap1_hit_detected = zap_read(0);
		// look for light in zapper, port 1
		// debug controller read code
		//  if (pad1_new & PAD_A)
		//  {
		//  	zap1_hit_detected = 1;
		//  }
	}
	if (trigger2_pulled == 1)
	{
		zap2_hit_detected = zap_read(1); // look for light in zapper, port 2

		// debug controller read code
		//  if (pad1_new & PAD_B)
		//  {
		//  	zap2_hit_detected = 1;
		//  }
	}
}

void move_balls(void)
{
	// offset = get_frame_count() & 3; // returns 0,1,2,3
	// offset = offset << 2;						// * 4, the size of the shuffle array
	for (index = 0; index < MAX_BALLS; ++index)
	{
		// index2 = shuffle_array[offset];
		// ++offset;
		// index2 = index; // <-- shortcut to keep the shuffling code in if we need it

		if (balls_type[index] == TURN_OFF)
			continue; // we found an empty spot

		move_ball();
	}
}

void move_ball(void)
{
	// bounce off ceiling
	if (balls_y[index] > TOP_BOUNDARY)
	{
		balls_y_direction[index] = GOING_UP;
	}
	if (balls_y[index] < BOTTOM_BOUNDARY)
	{
		balls_y_direction[index] = GOING_DOWN;
	}

	// move ball according to direction

	if (balls_x_direction[index] == GOING_LEFT)
	{
		balls_x[index] -= balls_x_speed[index];
	}
	else
	{
		balls_x[index] += balls_x_speed[index];
	}

	if (balls_y_direction[index] == GOING_UP)
	{
		balls_y[index] -= balls_y_speed[index];
	}
	else
	{
		balls_y[index] += balls_y_speed[index];
	}

	// check boundaries

	if (balls_x[index] < LEFT_BOUNDARY)
	{
		++player_1_score;
		balls_type[index] = TURN_OFF;
		zap1_cooldown = 0;
		zap2_cooldown = 0;
	}

	if (balls_x[index] > RIGHT_BOUNDARY)
	{
		++player_2_score;
		balls_type[index] = TURN_OFF;
		zap1_cooldown = 0;
		zap2_cooldown = 0;
	}
}
void clear_balls_and_hits()
{
	// clear all the types (used to draw)
	// clear how many hits
	for (index = 0; index < MAX_BALLS; ++index)
	{
		balls_type[index] = TURN_OFF;
		balls_hits[index] = 0;
	}
}

void new_ball(void)
{
	clear_balls_and_hits();

	// add a new big ball in slot 0

	index = 0;
	balls_type[index] = LARGE_BALL;

	balls_x[index] = MIDDLE_SCREEN;
	balls_y[index] = MIDDLE_SCREEN;

	balls_x_speed[index] = DEFAULT_X_SPEED;
	balls_y_speed[index] = DEFAULT_Y_SPEED;

	switch (get_frame_count() & 0b00000011)
	{
	case 0:
		balls_x_direction[index] = GOING_LEFT;
		balls_y_direction[index] = GOING_UP;
		break;
	case 1:
		balls_x_direction[index] = GOING_LEFT;
		balls_y_direction[index] = GOING_DOWN;
		break;
	case 2:
		balls_x_direction[index] = GOING_RIGHT;
		balls_y_direction[index] = GOING_DOWN;
		break;
	case 3:
		balls_x_direction[index] = GOING_RIGHT;
		balls_y_direction[index] = GOING_UP;
		break;
	default:
		break;
	}
}

void draw_box(void)
{

	temp1 = balls_x[index];
	temp2 = balls_y[index];

	// use if we want bigger targets
	// and need to realign them
	// switch (balls_type[index])
	// {
	// case SMALL_BALL:
	// 	temp1 = temp1 - 4;
	// 	temp2 = temp2 - 4;
	// 	break;
	// case MEDIUM_BALL:
	// 	break;
	// case LARGE_BALL:
	// 	temp1 = temp1 + 4;
	// 	temp2 = temp2 + 4;
	// 	break;
	// default:
	// 	break;
	// }

	switch (balls_type[index])
	{
	case LARGE_BALL:
		pointer2 = LargeBox;
		break;
	case MEDIUM_BALL:
		pointer2 = MediumBox;
		break;
	case SMALL_BALL:
		pointer2 = SmallBox;
		break;
	default:
		break;
	}
	oam_meta_spr(temp1, temp2, pointer2);
}

void draw_ball(void)
{
	temp1 = balls_x[index2]; // temp_x value
	temp2 = balls_y[index2]; // temp_y value

	//  the whole idea behind having sprites_type and sprites_anim is
	//  to have different anim frames, which we might want.
	switch (balls_type[index2])
	{
	case LARGE_BALL:
		pointer2 = LargeBall;
		break;
	case MEDIUM_BALL:
		pointer2 = MediumBall;
		break;
	case SMALL_BALL:
		pointer2 = SmallBall;
		break;
	default:
		pointer2 = SmallBall;
		break;
	}
	oam_meta_spr(temp1, temp2, pointer2);
}

void draw_balls(void)
{
	offset = get_frame_count() & 3; // returns 0,1,2,3
	offset = offset << 2;						// * 4, the size of the shuffle array
	for (index = 0; index < MAX_BALLS; ++index)
	{
		index2 = shuffle_array[offset];
		++offset;
		index2 = index; // <-- shortcut to keep the shuffling code in if we need it

		if (balls_type[index2] == TURN_OFF)
			continue; // we found an empty spot

		draw_ball();
	}
}

void draw_bg(void)
{
	ppu_off();	 // screen off
	oam_clear(); // clear all sprites

	set_mt_pointer(metatiles);
	set_data_pointer(level);
	memcpy(c_map, level, 240);

	// draw the tiles
	for (y = 0;; y += 0x20)
	{
		for (x = 0;; x += 0x20)
		{
			address = get_ppu_addr(0, x, y);
			index = (y & 0xf0) + (x >> 4);
			buffer_4_mt(address, index); // ppu_address, index to the data
			flush_vram_update2();
			if (x == 0xe0)
				break;
		}
		if (y == 0xe0)
			break;
	}

	ppu_on_all();
}

void draw_score(void)
{
	one_vram_buffer(player_1_score + 48, NTADR_A(3, 2));
	one_vram_buffer(player_2_score + 48, NTADR_A(26, 2));
}

void draw_cooldown(void)
{
	// player 1 cooldown
	index = 0;
	temp1 = zap1_cooldown >> 2;
	while (index < MAX_COOLDOWN >> 2)
	{
		if (index < temp1)
		{
			one_vram_buffer('l', NTADR_A(3 + index, 26));
		}
		else
		{
			one_vram_buffer(' ', NTADR_A(3 + index, 26));
		}

		index += 1;
	}

	// player 2 cooldown
	index = 0;
	temp1 = zap2_cooldown >> 2;
	while (index < MAX_COOLDOWN >> 2)
	{
		if (index < temp1)
		{
			one_vram_buffer('l', NTADR_A(26 - index, 26));
		}
		else
		{
			one_vram_buffer(' ', NTADR_A(26 - index, 26));
		}

		index += 1;
	}
}