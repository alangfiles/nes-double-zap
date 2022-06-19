// variables
#define DEFAULT_X_SPEED 300
#define DEFAULT_Y_SPEED 200
#define DEFAULT_SPEED_STEP_UP 400
#define DEFAULT_SPEED_STEP_DOWN 3
#define LEFT_BOUNDARY 0x0500
#define RIGHT_BOUNDARY 0xe000
#define TOP_BOUNDARY 0xb000
#define BOTTOM_BOUNDARY 0x2000
#define MIDDLE_SCREEN 0x7000

#define MAX_COOLDOWN 8
#define MAX_BALLS 4
#define TURN_OFF 0xff
#define NO_HITS 0xfe
#define MAX_HITS 10

#pragma bss-name(push, "ZEROPAGE")

unsigned char frames_to_wait = 0;	 // used to calibrate the screen for modern displays
unsigned char frames_to_read = 10; // how many frames to detect light for
unsigned char pad1_zapper;
unsigned char zap1_ready; // wait till it's 0
unsigned char zap1_hit_detected;
unsigned char pad2_zapper;
unsigned char zap2_ready; // wait till it's 0
unsigned char zap2_hit_detected;
unsigned char zap1_cooldown = 0;
unsigned char zap2_cooldown = 0;
unsigned char zap1_detected_in_wait = 0;
unsigned char zap2_detected_in_wait = 0;
unsigned char number_of_balls_active = 0;
unsigned char ball_index_hit = NO_HITS;

const unsigned char * pointer;
const unsigned char * pointer2;


#define MAX_BALLS 4
unsigned int balls_x[MAX_BALLS];
unsigned int balls_y[MAX_BALLS];
unsigned int balls_x_speed[MAX_BALLS];
unsigned int balls_y_speed[MAX_BALLS];
unsigned char balls_x_direction[MAX_BALLS];
unsigned char balls_y_direction[MAX_BALLS];
unsigned char balls_active[MAX_BALLS];
unsigned char balls_hits[MAX_BALLS];
//unsigned char sprites_actual_x[MAX_ROOM_SPRITES];
unsigned char balls_type[MAX_BALLS];

struct Base
{
	unsigned char x;
	unsigned char y;
	unsigned char width;
	unsigned char height;
	// unsigned char size;
	// unsigned char x_direction;
	// unsigned char y_direction;
	// unsigned char x_speed;
	// unsigned char y_speed;
	// unsigned char active;
};

struct Base Generic;

// debuging with pad
unsigned char pad1;
unsigned char pad1_new;

enum
{
	GOING_UP,
	GOING_DOWN
};
enum
{
	GOING_LEFT,
	GOING_RIGHT
};

unsigned char player_1_score = 0;
unsigned char player_2_score = 0;

unsigned char temp1;
unsigned char temp2;
unsigned char temp3;
unsigned char trigger1_pulled;
unsigned char trigger2_pulled;

unsigned char game_mode;
enum
{
	MODE_TITLE,
	MODE_GAME,
	MODE_PAUSE,
	MODE_END,
	MODE_GAME_OVER,
};

// room loader code
int address;
unsigned char x;
unsigned char y;
unsigned char index = 0;
unsigned char index2 = 0;

unsigned char offset;
const unsigned char shuffle_array[]={
0,1,2,3,
3,2,1,0,
0,2,1,3,
3,1,2,0
};

#pragma bss-name(push, "BSS")

unsigned char c_map[240]; // collision map

// PROTOTYPES
void move_ball(void);
void move_balls(void);
void new_ball(void);
void read_input_triggers(void);
void read_zapper_hits(void);
void handle_ball_hit(void);
void update_cooldown(void);
void draw_box(void);
void draw_ball(void);
void draw_balls(void);
void draw_bg(void);
void draw_score(void);
void draw_cooldown(void);
void trigger_pulled(void);
void clear_balls_and_hits(void);