#include <stdlib.h>
#include <string.h>

#include "pixler.h"
#include "common.h"

#define BG_COLOR 0x37
static const u8 PALETTE[] = {
	BG_COLOR, 0x26, 0x15, 0x04,
	BG_COLOR, 0x06, 0x16, 0x26,
	BG_COLOR, 0x09, 0x19, 0x29,
	BG_COLOR, 0x01, 0x11, 0x21,
	
	BG_COLOR, 0x26, 0x15, 0x04,
	BG_COLOR, 0x06, 0x16, 0x26,
	BG_COLOR, 0x09, 0x19, 0x29,
	BG_COLOR, 0x01, 0x11, 0x21,
};

Gamepad pad1, pad2;

void read_gamepads(void){
	pad1.prev = pad1.value;
	pad1.value = joy_read(0);
	pad1.press = pad1.value & (pad1.value ^ pad1.prev);
	pad1.release = pad1.prev & (pad1.value ^ pad1.prev);
	
	pad2.prev = pad2.value;
	pad2.value = joy_read(1);
	pad2.press = pad2.value & (pad2.value ^ pad2.prev);
	pad2.release = pad2.prev & (pad2.value ^ pad2.prev);
}

void wait_noinput(void){
	while(joy_read(0) || joy_read(1)) px_wait_nmi();
}

static void darken(register const u8* palette, u8 shift){
	for(idx = 0; idx < 32; idx++){
		ix = palette[idx];
		ix -= shift << 4;
		if(ix > 0x40 || ix == 0x0D) ix = 0x1D;
		px_buffer_set_color(idx, ix);
	}
}

void fade_from_black(const u8* palette, u8 delay){
	darken(palette, 4);
	px_wait_frames(delay);
	darken(palette, 3);
	px_wait_frames(delay);
	darken(palette, 2);
	px_wait_frames(delay);
	darken(palette, 1);
	px_wait_frames(delay);
	darken(palette, 0);
}

void meta_spr(u8 x, u8 y, u8 pal, const u8* data);
void meta_spr2(u8 x, s16 y, bool flipx, const u8* data);

#define BOBY_META_N(_idx_) \
	-8, -16, 0x00 + (2*_idx_), 0, \
	 0, -16, 0x01 + (2*_idx_), 0, \
	-8,  -8, 0x3A + (2*_idx_), 0, \
	 0,  -8, 0x3B + (2*_idx_), 0, \
	 128, \
	

static const u8 _BOBY_META[] = {
	BOBY_META_N(0x00)
	BOBY_META_N(0x01)
	BOBY_META_N(0x02)
	BOBY_META_N(0x03)
	BOBY_META_N(0x04)
	BOBY_META_N(0x05)
	BOBY_META_N(0x06)
	BOBY_META_N(0x07)
	BOBY_META_N(0x08)
	BOBY_META_N(0x09)
	BOBY_META_N(0x0A)
	BOBY_META_N(0x0B)
	BOBY_META_N(0x0C)
	BOBY_META_N(0x0D)
	BOBY_META_N(0x0E)
	BOBY_META_N(0x0F)
	BOBY_META_N(0x10)
	BOBY_META_N(0x11)
	BOBY_META_N(0x12)
	BOBY_META_N(0x13)
	BOBY_META_N(0x14)
	BOBY_META_N(0x15)
	BOBY_META_N(0x16)
	BOBY_META_N(0x17)
	BOBY_META_N(0x18)
	BOBY_META_N(0x19)
	BOBY_META_N(0x1A)
	BOBY_META_N(0x1B)
	BOBY_META_N(0x1C)
	BOBY_META_N(0x1D)
};

static const u8* BOBY_IDLE[] = {
	_BOBY_META +  (  0/16)*17,
	_BOBY_META +  (  1/16)*17,
	_BOBY_META +  (  2/16)*17,
	_BOBY_META +  (  0/16)*17,
	_BOBY_META +  (  1/16)*17,
	_BOBY_META +  (  2/16)*17,
};
static const u8 BOBY_IDLE_LEN = sizeof(BOBY_IDLE)/sizeof(*BOBY_IDLE);

static const u8* BOBY_RUN[] = {
	_BOBY_META + ( 48/16)*17,
	_BOBY_META + ( 64/16)*17,
	_BOBY_META + ( 80/16)*17,
	_BOBY_META + ( 96/16)*17,
	_BOBY_META + (112/16)*17,
	_BOBY_META + ( 80/16)*17,
};
static const u8 BOBY_RUN_LEN = sizeof(BOBY_RUN)/sizeof(*BOBY_RUN);

static const u8* BOBY_JUMP[] = {
	_BOBY_META + (  0/16)*17,
	_BOBY_META + (128/16)*17,
	_BOBY_META + (144/16)*17,
	_BOBY_META + (160/16)*17,
	_BOBY_META + (176/16)*17,
	_BOBY_META + (192/16)*17,
	_BOBY_META + (208/16)*17,
	_BOBY_META + (224/16)*17,
	_BOBY_META + (240/16)*17,
	_BOBY_META + (256/16)*17,
	_BOBY_META + (272/16)*17,
	_BOBY_META + (128/16)*17,
};
static const u8 BOBY_JUMP_LEN = sizeof(BOBY_JUMP)/sizeof(*BOBY_JUMP);

static const u8* BOBY_CROUCH[] = { // 25 - 29
	_BOBY_META + (  0/16)*17,
	_BOBY_META + (288/16)*17,
	_BOBY_META + (304/16)*17,
	_BOBY_META + (272/16)*17,
	_BOBY_META + (128/16)*17,
};
static const u8 BOBY_CROUCH_LEN = sizeof(BOBY_CROUCH)/sizeof(*BOBY_CROUCH);

static const u8* BOBY_DIVE[] = { // 34 - 37
	_BOBY_META + (224/16)*17,
	_BOBY_META + (320/16)*17,
	_BOBY_META + (336/16)*17, // loop these last two
	_BOBY_META + (352/16)*17,
};
static const u8 BOBY_DIVE_LEN = sizeof(BOBY_DIVE)/sizeof(*BOBY_DIVE);

typedef struct {
	long px, py;
	short vx, vy;
	bool facingLeft;
	u8 pallete;
	u8 tileData;
	u8* curSprite;
	u8 curAnimLen;
	short x, y;
} Player;

Player player = {48 << 8, 400l << 8};

#define GRAVITY 16
#define MAX_FALL_SPEED (2 << 8)
#define SUPER_FALL_SPEED (4 << 8)
#define JUMPSPEED -550
#define JUMPTIMERMAX 31

static bool collision_check(short x, short y){
	// get tile we are colliding with 
	player.tileData = MAP_SPLASH[32*(y>>3)+(x>>3)];

	// if we are colliding, change the pallete
	if (player.tileData != 0) {
		return true;
	}
	
	return false;
}

typedef enum {
	JUMP_READY,
	JUMP_BOUNCE,
	JUMP_BOUNCED,
} JumpState;

bool onFloor = false;
JumpState jumpState;
int peakYPos = 0;
int flop =  true;
int bounceTimer = 0;
int flip = false;
static void update_player(){
	bool walking = false;
	// ACTUAL INPUT
	if(JOY_LEFT (pad1.value)) { player.px -= 1 << 8; walking = true; player.facingLeft = true; }
	if(JOY_RIGHT(pad1.value)) { player.px += 1 << 8; walking = true; player.facingLeft = false; }

	if (onFloor) {
		if (walking) {
			meta_spr2(player.x, player.y, player.facingLeft, BOBY_RUN[(px_ticks/4) % BOBY_RUN_LEN]);
		}
		else {
			meta_spr2(player.x, player.y, player.facingLeft, BOBY_IDLE[(px_ticks/4) % BOBY_IDLE_LEN]);
		}
	}
	else {
		if (jumpState == JUMP_BOUNCE) {
			meta_spr2(player.x, player.y, player.facingLeft, BOBY_JUMP[(px_ticks/4) % BOBY_JUMP_LEN/2]);
		}
		else {
			meta_spr2(player.x, player.y, player.facingLeft, BOBY_JUMP[(px_ticks/4) % BOBY_JUMP_LEN/2]);
		}
	}
	
	// We are not on floor
	if (!collision_check(player.x,player.y+1)) {
		if(bounceTimer <= 0){
			player.vy += GRAVITY;
		}
		if (JOY_DOWN(pad1.press)) {
			player.vy = -JUMPSPEED;
			switch(jumpState){
				case JUMP_READY:
					bounceTimer = 0;
					break;
				case JUMP_BOUNCE:
					// bounce is already started, ignore.
					break;
				case JUMP_BOUNCED:
					// second bounce, add pentalty so you can't just chain bouncing higher and higher
					bounceTimer = -10;
					break;
			}
			jumpState = JUMP_BOUNCE;
		}

		if (jumpState == JUMP_BOUNCE) {
			if(player.vy > SUPER_FALL_SPEED) player.vy = SUPER_FALL_SPEED;
		}
		else {
			if(player.vy > MAX_FALL_SPEED) player.vy = MAX_FALL_SPEED;
		}
	}
	// We are on floor
	else {
		if (JOY_BTN_B(pad1.press)){
			flip = false;
			onFloor = false;
			player.vy = JUMPSPEED;
			sound_play(SOUND_JUMP);
		}
	}
	
	// apply velocity to position
	player.px += player.vx;
	player.py += player.vy;
	
	// pallete collision debug
	player.pallete = 2;

	// update pixel positon x
	player.x = player.px >> 8;
	if (collision_check(player.x,player.y)) {
		int deltaX = player.x % 8;
		if (deltaX < 4) player.px -= (deltaX+1) << 8;
		else player.px += (8-deltaX) << 8;
		player.x = player.px >> 8;
		player.vx = 0;
		player.pallete = 3;
	}
	
	if(jumpState == JUMP_BOUNCE && player.vy >= MAX_FALL_SPEED){
		bounceTimer += 1;
	}
	if(jumpState == JUMP_BOUNCED && bounceTimer > 0){
		bounceTimer -= 1;
		player.vy = JUMPSPEED;
	}

	// update pixel position y
	player.y = player.py >> 8;
	if (collision_check(player.x,player.y)) {
		int deltaY = player.y % 8;
		if (deltaY < 4) player.py -= (deltaY+1) << 8;
		else player.py += (8-deltaY) << 8;
		player.y = player.py >> 8;
		onFloor = true;
		if (jumpState == JUMP_BOUNCE) {
			player.vy = JUMPSPEED;
			jumpState = JUMP_BOUNCED;
			// if(bounceTimer > 20) bounceTimer -= 20; else bounceTimer = 0;
			sound_play(SOUND_JUMP);
		}
		else if (jumpState == JUMP_BOUNCED) {
			player.vy = JUMPSPEED/3;
			jumpState = JUMP_READY;
			sound_play(SOUND_JUMP);
		}
		else {
		 	player.vy = 0;
		}
		
		player.pallete = 3;
		flop = true;
	}

	// draw the tile hex on the screen for debugging;
	px_debug_hex_addr = NT_ADDR(0,2,2);
	px_debug_hex(flip);
}

static void load_map(){
	// Load the splash tilemap into nametable 0.
	px_addr(NT_ADDR(0, 0, 0));
	px_blit(0x3C0, MAP_SPLASH + 0x000);
	px_blit(0x040, MAP_SPLASH + 0x780);
	px_addr(NT_ADDR(2, 0, 0));
	px_blit(0x3C0, MAP_SPLASH + 0x3C0);
	// px_blit(0x040, MAP_SPLASH + 0x7C0);
}

static void splash_screen(void){
	px_ppu_sync_disable();{
		load_map();
	} px_ppu_sync_enable();
	
	// music_play(0);
	
	fade_from_black(PALETTE, 4);
	
	while(true){
		px_profile_start();
		read_gamepads();
		
		update_player();
		
		// Draw a sprite.
		//meta_spr2(player.x, player.y, player.facingLeft, player.curSprite[(px_ticks/8) % player.curAnimLen]);
		
		{
			int scroll = player.y - 128;
			if(scroll < 0) scroll = 0;
			if(scroll > 240) scroll = 240;
			PX.scroll_y = scroll;
		}
		
		px_profile_end();
		px_spr_end();
		px_wait_nmi();
	}
	
	splash_screen();
}

void main(void){
	// Set up CC65 joystick driver.
	joy_install(nes_stdjoy_joy);
	
	// Set which tiles to use for the background and sprites.
	px_bg_table(0);
	px_spr_table(1);
	
	// Not using bank switching, but a good idea to set a reliable value at boot.
	px_uxrom_select(0);
	
	// Black out the palette.
	for(idx = 0; idx < 32; idx++) px_buffer_set_color(idx, 0x1D);
	px_wait_nmi();
	
	// Decompress the tileset into character memory.
	px_lz4_to_vram(CHR_ADDR(0, 0), CHR0);
	px_lz4_to_vram(CHR_ADDR(1, 0), BOBY);
	
	music_init(&MUSIC);
	sound_init(&SOUNDS);
	// music_play(0);
	
	// Jump to the splash screen state.
	splash_screen();
}
