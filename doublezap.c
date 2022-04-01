// todo:
// [] change angle of the ball (y and x probably) whenever it's hit close to the goal line
// [] fix background

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


	ppu_on_all(); // turn on screen

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

			if (ball_active)
			{
				move_ball();
				draw_ball();
				if (trigger1_pulled || trigger2_pulled)
				{
					// idk if this'll work to delay the drawing
					index = 0;
					while (index < frames_to_wait)
					{
						ppu_wait_nmi();
						++index;
					}

					// trigger pulled, play bang sound
					// sfx_play(0, 0);

					// bg off, project white boxes
					oam_clear();
					draw_box();			// redraw the star as a box
					ppu_mask(0x16); // BG off, won't happen till NEXT frame

					ppu_wait_nmi(); // wait till the top of the next frame
					// this frame will display no BG and a white box

					oam_clear();		// clear the NEXT frame
					draw_ball();		// draw a star on the NEXT frame
					ppu_mask(0x1e); // bg on, won't happen till NEXT frame

					read_zapper_hits();
					update_ball_movement(); // based off zapper hit data
					// if hit failed, it should have already ran into the next nmi
				}
			}
			else if (ball_wait)
			{
				--ball_wait;
			}
			else
			{
				new_ball();
			}
			gray_line();
		}
	}
}

void update_cooldown(void)
{
	if(zap1_cooldown > 0){
		--zap1_cooldown;
	}
	if(zap2_cooldown > 0){
		--zap2_cooldown;
	}
}

void update_ball_movement(void)
{
	if (zap1_hit_detected == 1)
	{
		ball_x_direction = GOING_RIGHT;
	}
	if (zap2_hit_detected == 1)
	{
		ball_x_direction = GOING_LEFT;
	}

	if (zap1_hit_detected || zap2_hit_detected) // if it's hit update the speed
	{
		ball_x_speed += DEFAULT_SPEED_STEP;
		if (get_frame_count() & 0x01 == 1)
		{
			ball_y_speed -= DEFAULT_SPEED_STEP / 4;
		}
		else
		{
			ball_y_speed += DEFAULT_SPEED_STEP;
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

	// for debug, testing just a controller:
	pad1 = pad_poll(0);
	pad1_new = get_pad_new(0);

	if ((pad1_new & PAD_A) && zap1_cooldown == 0) //((pad1_zapper) && (zap1_ready));
	{
		trigger1_pulled = 1;
		zap1_cooldown = MAX_COOLDOWN;
	}
	if ((pad1_new & PAD_B) && zap2_cooldown == 0) //((pad2_zapper) && (zap2_ready));
	{
		trigger2_pulled = 1;
		zap2_cooldown = MAX_COOLDOWN;
	}
}

void read_zapper_hits(void)
{
	// only reads the hit for the zapper pulled
	// this should be the read code:
	if (trigger1_pulled == 1)
	{
		//  zap1_hit_detected = zap_read(0); // look for light in zapper, port 1
		if (pad1_new & PAD_A) //this is debug code
		{
			zap1_hit_detected = 1;
		}
	}
	if (trigger2_pulled == 1)
	{
		//  zap2_hit_detected = zap_read(1); // look for light in zapper, port 2
		if (pad1_new & PAD_B)//this is debug code
		{
			zap2_hit_detected = 1;
		}
	}

}

void move_ball(void)
{
	// bounce off ceiling
	if (ball_y > TOP_BOUNDARY)
	{
		ball_y_direction = GOING_UP;
	}
	if (ball_y < BOTTOM_BOUNDARY)
	{
		ball_y_direction = GOING_DOWN;
	}

	// move ball according to direction

	if (ball_x_direction == GOING_LEFT)
	{
		ball_x -= ball_x_speed;
	}
	else
	{
		ball_x += ball_x_speed;
	}

	if (ball_y_direction == GOING_UP)
	{
		ball_y -= ball_y_speed;
	}
	else
	{
		ball_y += ball_y_speed;
	}

	// check boundaries

	if (ball_x < LEFT_BOUNDARY)
	{
		++player_1_score;
		ball_active = 0;
		ball_wait = 20;
		zap1_cooldown = 0;
		zap2_cooldown = 0;
	}

	if (ball_x > RIGHT_BOUNDARY)
	{
		++player_2_score;
		ball_active = 0;
		ball_wait = 20;
		zap1_cooldown = 0;
		zap2_cooldown = 0;
	}
}

void new_ball(void)
{
	ball_active = 1;
	ball_x = MIDDLE_SCREEN; // should give 0x4000-0xbf80
	ball_y = MIDDLE_SCREEN; // int

	ball_x_speed = DEFAULT_X_SPEED; // 0 is stopped, 400 is fast
	ball_y_speed = DEFAULT_Y_SPEED;

	switch (get_frame_count() & 0b00000011)
	{
	case 0:
		ball_x_direction = GOING_LEFT;
		ball_y_direction = GOING_UP;
		break;
	case 1:
		ball_x_direction = GOING_LEFT;
		ball_y_direction = GOING_DOWN;
		break;
	case 2:
		ball_x_direction = GOING_RIGHT;
		ball_y_direction = GOING_DOWN;
		break;
	case 3:
		ball_x_direction = GOING_RIGHT;
		ball_y_direction = GOING_UP;
		break;
	default:
		break;
	}
}

void draw_box(void)
{
	temp1 = high_byte(ball_x);
	temp2 = high_byte(ball_y);
	oam_meta_spr(temp1, temp2, WhiteBox);
}

void draw_ball(void)
{
	temp1 = high_byte(ball_x);
	temp2 = high_byte(ball_y);

	oam_meta_spr(temp1, temp2, AlanHead);
}

void draw_bg(void)
{
	ppu_off();				 // screen off
	oam_clear();			 // clear all sprites

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
	// multi_vram_buffer_vert(lines, sizeof(lines) - 1, NTADR_A(3, 8));
	// multi_vram_buffer_vert(lines, sizeof(lines) - 1, NTADR_A(26, 8));
}

void draw_cooldown(void)
{
	//player 1 cooldown
	index = 0;
	temp1 = zap1_cooldown>>2;
	while(index < MAX_COOLDOWN>>2)
	{
		if(index < temp1) 
		{
			one_vram_buffer('|', NTADR_A(3+index, 26));
		} 
		else {
			one_vram_buffer(' ', NTADR_A(3+index, 26));
		}
		
		index += 1;
	}

	//player 2 cooldown
	index = 0;
	temp1 = zap2_cooldown>>2;
	while(index < MAX_COOLDOWN>>2)
	{
		if(index < temp1)  
		{
			one_vram_buffer('|', NTADR_A(26-index, 26));
		} 
		else {
			one_vram_buffer(' ', NTADR_A(26-index, 26));
		}
		
		index += 1;
	}

}