//variables
#define DEFAULT_X_SPEED 300
#define DEFAULT_Y_SPEED 200
#define DEFAULT_SPEED_STEP 40
#define LEFT_BOUNDARY 0x0500
#define RIGHT_BOUNDARY 0xe000
#define TOP_BOUNDARY 0xb000
#define BOTTOM_BOUNDARY 0x2000
#define MIDDLE_SCREEN 0x7000

#define MAX_COOLDOWN 32

#pragma bss-name(push, "ZEROPAGE")

unsigned char frames_to_wait = 5; //used to calibrate the screen for modern displays
unsigned char pad1_zapper;
unsigned char zap1_ready; //wait till it's 0
unsigned char zap1_hit_detected;
unsigned char pad2_zapper;
unsigned char zap2_ready; //wait till it's 0
unsigned char zap2_hit_detected;
unsigned char zap1_cooldown = 0;
unsigned char zap2_cooldown = 0;

//debuging with pad
unsigned char pad1;
unsigned char pad1_new;


enum{
	GOING_UP, GOING_DOWN
};
enum{
	GOING_LEFT, GOING_RIGHT
};

unsigned char ball_x_direction;
unsigned char ball_y_direction = GOING_UP;
unsigned char ball_active;
unsigned int ball_x;
unsigned int ball_y;
unsigned int ball_x_speed;
unsigned int ball_y_speed;
unsigned char ball_wait = 0;
unsigned char player_1_score = 0;
unsigned char player_2_score = 0;

unsigned char temp1;
unsigned char temp2;
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

#pragma bss-name(push, "BSS")

unsigned char c_map[240];// collision map

// PROTOTYPES
void move_ball(void);
void new_ball(void);
void read_input_triggers(void);
void read_zapper_hits(void);
void update_ball_movement(void);
void update_cooldown(void);
void draw_box(void);
void draw_ball(void);
void draw_bg(void);
void draw_score(void);
void draw_cooldown(void);
