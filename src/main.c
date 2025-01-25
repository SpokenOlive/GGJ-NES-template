#include <stdlib.h>
#include <string.h>

#include "pixler.h"
#include "common.h"

#define BG_COLOR 0x31
static const u8 PALETTE[] = {
	BG_COLOR, 0x00, 0x10, 0x20,
	BG_COLOR, 0x06, 0x16, 0x26,
	BG_COLOR, 0x09, 0x19, 0x29,
	BG_COLOR, 0x01, 0x11, 0x21,
	
	BG_COLOR, 0x00, 0x10, 0x20,
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
static const u8 META[] = {
	-8, -8, 0xD0, 0,
	 0, -8, 0xD1, 0,
	-8,  0, 0xD2, 0,
	 0,  0, 0xD2, 0,
	128,
};

typedef struct {
	long px, py;
	long nx, ny;
	short vx, vy;
	u8 pallete;
	u8 tileData;
	short x, y;
} Player;

Player player = {32 << 8, 32 << 8};

#define GRAVITY 64
#define MAX_FALL_SPEED (4 << 8)

static bool collision_check(short x, short y){
	// get tile we are colliding with 
	player.tileData = MAP_SPLASH[32*(y>>3)+(x>>3)];

	// if we are colliding, change the pallete
	if (player.tileData != 0) {
		return true;
	}
	
	return false;
}

bool onFloor = false;

static void update_player(){
	// ACTUAL INPUT
	if(JOY_LEFT (pad1.value)) player.px -= 1 << 8;
	if(JOY_RIGHT(pad1.value)) player.px += 1 << 8;
	//if(JOY_DOWN (pad1.value)) player.py += 1 << 8;
	//if(JOY_UP   (pad1.value)) player.py -= 1 << 8;
	if(JOY_BTN_A(pad1.press)) sound_play(SOUND_JUMP);

	if(pad1.press & JOY_BTN_A_MASK & player.vy == 0){
		player.vy = -1000;
	}
	
	// Apply gravity and clamp
	if (!collision_check(player.x,player.y+1)) {
		player.vy += GRAVITY;
		if(player.vy > MAX_FALL_SPEED) player.vy = MAX_FALL_SPEED;
	}
	
	// apply velocity to position
	player.px += player.vx;
	player.py += player.vy;

	// don't fall through the bottom of the screen
	if(player.py > (239l << 8)){ // l makes it a LONG int
		player.py = (239l << 8);
		if(player.vy > 0) player.vy = 0;
	}
	
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

	// update pixel position y
	player.y = player.py >> 8;
	if (collision_check(player.x,player.y)) {
		int deltaY = player.y % 8;
		if (deltaY < 4) player.py -= (deltaY+1) << 8;
		else player.py += (8-deltaY) << 8;
		player.y = player.py >> 8;
		player.vy = 0;
		player.pallete = 3;
	}

	// draw the tile hex on the screen for debugging;
	px_debug_hex_addr = NT_ADDR(0,2,2);
	px_debug_hex(player.vy);
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
	static s16 sin = 0, cos = 0x3FFF;
	px_ppu_sync_disable();{
		load_map();
	} px_ppu_sync_enable();
	
	// music_play(0);
	
	fade_from_black(PALETTE, 4);
	
	while(true){
		read_gamepads();
		
		px_profile_start();
		update_player();
		px_profile_end();
		
		// Draw a sprite.
		meta_spr(player.x, player.y, player.pallete, META);
		
		// PX.scroll_y = 480 + (sin >> 9);
		// sin += cos >> 6;
		// cos -= sin >> 6;
		
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
	px_spr_table(0);
	
	// Not using bank switching, but a good idea to set a reliable value at boot.
	px_uxrom_select(0);
	
	// Black out the palette.
	for(idx = 0; idx < 32; idx++) px_buffer_set_color(idx, 0x1D);
	px_wait_nmi();
	
	// Decompress the tileset into character memory.
	px_lz4_to_vram(CHR_ADDR(0, 0), CHR0);
	
	music_init(&MUSIC);
	sound_init(&SOUNDS);
	// music_play(0);
	
	// Jump to the splash screen state.
	splash_screen();
}
