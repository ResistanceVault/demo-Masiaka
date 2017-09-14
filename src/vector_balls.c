#include <genesis.h>
#include <gfx.h>
#include "ball_coords.h"
#include "quicksort.h"
#include "demo_strings.h"
#include "writer.h"
#include "transition_helper.h"
#include "sine_scroll.h"
#include "music.h"

#define	MAX_VECTOR_BALL 256
#define BALL_COUNT grid_cube_small_VTX_COUNT
#define VECTOR_BALL_ARRAY vb_grid_cube_small_vertex_pos
#define VBALL_DISTANCE 1100

#define VBALL_PHASE_INTRO_SCROLL		0
#define VBALL_PHASE_INTRO_LOGO_FADEOUT	1
#define VBALL_PHASE_INTRO_FADE			2
#define VBALL_PHASE_BEGIN				3
#define VBALL_PHASE_FADETOWHITE			4
#define VBALL_PHASE_FADEIN				5
#define VBALL_PHASE_RUN					6
#define VBALL_PHASE_FADEOUT				7
#define VBALL_NEXT_OBJECT				8
#define VBALL_PHASE_QUIT				9

#define VBALL_X_SCREEN (((320 - 32) >> 1) + 0x80)
#define VBALL_Y_SCREEN (((224 - 32) >> 1) + 0x80)
#define VBALL_X_SCREEN_SHADOW (VBALL_X_SCREEN + 0x30)
#define VBALL_Y_SCREEN_SHADOW (VBALL_Y_SCREEN + 0x60)

extern u16 vramIndex;
// extern u16 fontIndex;
extern u8 framerate;

static u16 loop, shadow_idx, j;
static 	u16 zsort_switch = 0;
static 	Sprite *sprites[MAX_VECTOR_BALL];
static 	struct  QSORT_ENTRY vball_zsort[MAX_VECTOR_BALL];
static 	short xc, yc;
static	short rx, ry;
static 	u8 vball_phase;
static 	u16 vball_timer;
static	u8 ball_count;
static	Vect3D_f16 *vector_ball_array;

static short x, y, z;
static Vect3D_f16 _vtx, t_vtx[MAX_VTX_COUNT];
static fix16 _cosx, _sinx, _cosy, _siny, cs, cc, ss, sc;

// #ifdef SEEMS_2_CRASH_FIRE_FX
// static s16 sine_scroll[256];
static s16 ramped_sine_scroll[256];
static s16 sine_ramp; 
// #endif

static u16 tileIndexes[64];

NOINLINE void fastVectorBallFX(Sprite **prev_sprites)
{
	u16 angle, sec_angle, sec_angle_step;
	u8 object_idx;
	Object vb_objects[BALL_COUNT];
	u32 music_sync = 0;
	u16 vramIndex_mem;

	inline auto void drawVectorBalls(u16 constant_angle, u16 accel_angle)
	{
		rx = constant_angle;
		ry = constant_angle >> 1;

		xc = cosFix16(rx << 3) << 2;
		yc = sinFix16(rx << 2);

		rx = constant_angle + (accel_angle >> 3);
		ry = constant_angle + (accel_angle >> 2);

		/* precalculate the rotation */
		_cosx = cosFix16(rx);
		_sinx = sinFix16(rx);
		_cosy = cosFix16(ry);
		_siny = sinFix16(ry);
		cs = fix16Mul(_cosx, _siny);
		ss = fix16Mul(_siny, _sinx);
		cc = fix16Mul(_cosx, _cosy);
		sc = fix16Mul(_sinx, _cosy);

		/* rotate the vector balls */
		for(loop = 0, shadow_idx = ball_count; loop < ball_count; loop++, shadow_idx++)
		{
			// The balls are processed by Z-order
			// 3D transformation (rotation on X and Y axis)
			j = vball_zsort[loop].index;

			_vtx = vector_ball_array[j];

		    t_vtx[j].x = fix16Add(fix16Mul(_vtx.x, _sinx), fix16Mul(_vtx.y, _cosx));
		    t_vtx[j].y = fix16Sub(fix16Mul(_vtx.x, cs), fix16Add(fix16Mul(_vtx.y, ss), fix16Mul(_vtx.z, _cosy)));
		    t_vtx[j].z = fix16Sub(fix16Mul(_vtx.x, cc), fix16Mul(_vtx.y, sc) - fix16Mul(_vtx.z, _siny));

		    t_vtx[j].x += xc;
		    t_vtx[j].y += yc;

			//	Isometric projection
		    x = t_vtx[j].x + (t_vtx[j].z >> 3);
		    y = t_vtx[j].y;

		    x >>= 3;
		    y >>= 3;

			z = t_vtx[j].z + FIX16(1.5);
			if (z < FIX16(0.0))
				z = FIX16(0.0);

			z >>= 6;

			if (z > 8)
				z = 8;

			/* Vector ball */
	        sprites[loop]->x = VBALL_X_SCREEN + x;
	        sprites[loop]->y = VBALL_Y_SCREEN + y;
	        sprites[loop]->status = sprites[loop]->status | 0x0002;

		    /* shadow */
	        sprites[shadow_idx]->x = VBALL_X_SCREEN_SHADOW + x;
	        sprites[shadow_idx]->y = VBALL_Y_SCREEN_SHADOW + (y >> 2);
	        sprites[shadow_idx]->status = sprites[loop]->status | 0x0002;

		    if (zsort_switch & 0x1)
		    {
				SPR_setVRAMTileIndex(sprites[loop], tileIndexes[z]);
				sprites[loop]->status |= 0x0040;
			}
			else
			{
				if (!(zsort_switch & 0x3))
				{
					SPR_setVRAMTileIndex(sprites[shadow_idx], tileIndexes[z + 9]);
					sprites[shadow_idx]->status |= 0x0040;
				}
		    }
		}

		/* Z-sort the vector balls */
		if (zsort_switch == 0)
		{
			for(loop = 0; loop < ball_count; loop++)
			{
			    //	Fill the sort table
			    vball_zsort[loop].index = loop;
			    vball_zsort[loop].value = t_vtx[loop].z;
			}

			QuickSort(ball_count, vball_zsort);
		}

		//	Count 16 frames until the next depth sort
		zsort_switch++;
		zsort_switch &= 0x1F;

		SPR_update(sprites, ball_count << 1);
	}

	auto void clearTitleSprite(void)
	{
		SPR_clear();
		SPR_end();
		VDP_waitVSync();
		DMA_flushQueue();
	}

	auto void initVectorBallsSprites(void)
	{
		SPR_init(0,0,0);

	    for(loop = 0; loop < ball_metal.animations[0]->numFrame; loop++)
	    {
	        TileSet* tileset = ball_metal.animations[0]->frames[loop]->tileset;

	        VDP_loadTileSet(tileset, vramIndex, TRUE);
	        tileIndexes[loop] = vramIndex;
	        vramIndex += tileset->numTile;
	    }

	    // VDP_waitVSync();

		/*	Initialize the needed amount of sprites */
		for(loop = 0; loop < ball_count; loop++)
		{
		    sprites[loop] = SPR_addSprite(&ball_metal, 0, 0, TILE_ATTR_FULL(PAL2, TRUE, FALSE, FALSE, 0));
			SPR_setAlwaysVisible(sprites[loop], TRUE);
			SPR_setAutoTileUpload(sprites[loop], FALSE);
			sprites[loop]->data = (u32) &vb_objects[loop];
			SPR_setVRAMTileIndex(sprites[loop], tileIndexes[loop & 0x7]);			
		}

	    for(loop = 0; loop < ball_shadow.animations[0]->numFrame; loop++)
	    {
	        TileSet* tileset = ball_shadow.animations[0]->frames[loop]->tileset;

	        VDP_loadTileSet(tileset, vramIndex, TRUE);
	        tileIndexes[loop + ball_metal.animations[0]->numFrame] = vramIndex;
	        vramIndex += tileset->numTile;
	    } 	

		for(loop = ball_count; loop < ball_count << 1; loop++)
		{
		    sprites[loop] = SPR_addSprite(&ball_shadow, 0, 0, TILE_ATTR_FULL(PAL2, TRUE, FALSE, FALSE, 0));
			SPR_setAlwaysVisible(sprites[loop], TRUE);
			SPR_setAutoTileUpload(sprites[loop], FALSE);
			sprites[loop]->data = (u32) &vb_objects[loop];
			SPR_setVRAMTileIndex(sprites[loop], TILE_ATTR_FULL(PAL3, TRUE, FALSE, FALSE, tileIndexes[8 + (loop & 0x7)]));
		}
	}

	/*
		Screen setup
	*/

	VDP_setEnable(0);
	
	VDP_setScreenWidth320();
	// RSE_changeResolution(320);

	/*fontIndex = */RSE_writerSetup();
	if (framerate == 60)
		demo_strings = demo_strings_NTSC;
	else
		demo_strings = demo_strings_PAL;

	// RSE_turn_screen_to_black();
	VDP_setPlanSize(64, 64);
	VDP_setWindowAddress(0xE000 - 0x40);
	VDP_setWindowHPos(FALSE, 1);
	VDP_setWindowVPos(FALSE, 0);	
	VDP_clearPlan(PLAN_A, TRUE);
	SPR_update(1);
	VDP_clearPlan(PLAN_B, TRUE);
	SPR_update(1);
	VDP_clearPlan(PLAN_WINDOW, TRUE);
	SPR_update(1);

	VDP_setScrollingMode(HSCROLL_PLANE, VSCROLL_PLANE);
	VDP_setVerticalScroll(PLAN_A, -64);
	VDP_setVerticalScroll(PLAN_B, 0);
	VDP_setVerticalScrollTile(PLAN_A, 0, (s16*)palette_black, 20, FALSE);
	VDP_setVerticalScrollTile(PLAN_B, 0, (s16*)palette_black, 20, FALSE);
	VDP_setHilightShadow(1);

	SPR_update(1);
	// SPR_init(0,0,0);
	// vramIndex = fontIndex;

	object_idx = 0;
	ball_count = BALL_COUNT;
	vector_ball_array = (Vect3D_f16 *)VECTOR_BALL_ARRAY;

	VDP_drawImageEx(PLAN_B, &checkboard_0, TILE_ATTR_FULL(PAL0, TRUE, FALSE, FALSE, vramIndex), 0, (224 - 96) >> 3, FALSE, TRUE);
	vramIndex += checkboard_0.tileset->numTile;

	VDP_drawImageEx(PLAN_B, &checkboard_1, TILE_ATTR_FULL(PAL0, TRUE, FALSE, FALSE, vramIndex), 80 >> 3, (224 - 96) >> 3, FALSE, TRUE);
	vramIndex += checkboard_1.tileset->numTile;

	VDP_drawImageEx(PLAN_B, &checkboard_2, TILE_ATTR_FULL(PAL0, TRUE, FALSE, FALSE, vramIndex), 160 >> 3, (224 - 96) >> 3, FALSE, TRUE);
	vramIndex += checkboard_2.tileset->numTile;

	VDP_drawImageEx(PLAN_B, &checkboard_3, TILE_ATTR_FULL(PAL0, TRUE, FALSE, FALSE, vramIndex), 240 >> 3, (224 - 96) >> 3, FALSE, TRUE);
	vramIndex += checkboard_3.tileset->numTile;

	VDP_drawImageEx(PLAN_B, &sky, TILE_ATTR_FULL(PAL1, TRUE, FALSE, FALSE, vramIndex), 0, ((224 - 96 - 72) >> 3), FALSE, TRUE);
	vramIndex += sky.tileset->numTile;

	vramIndex_mem = vramIndex;

	/* Weird technique to "clear" the whole screen
	with the TILE_SYSTEMINDEX tile, to avoid random
	screen corruption */
	{
		u16 row, col, max_col;
		max_col = VDP_getPlanWidth(); 
		for(row = 0; row < (224 - 96 - 72) >> 3; row++)
			for(col = 0; col < max_col; col++)
				VDP_setTileMapXY(PLAN_B, TILE_ATTR_FULL(PAL1, TRUE, FALSE, FALSE, TILE_SYSTEMINDEX), col, row);
	}

	DMA_flushQueue();
	VDP_setEnable(1);

	angle = 0;
	sec_angle = 0;
	sec_angle_step = 0;

	vball_phase = VBALL_PHASE_INTRO_SCROLL;
	vball_timer = 0;

	VDP_fadePalTo(PAL0, checkboard_0.palette->data, RSE_FRAMES(8), TRUE);

	while(vball_phase < VBALL_PHASE_QUIT)
	{
		VDP_waitVSync();
		DMA_flushQueue();
		music_sync = music_getElapsed();

		angle++;

		if (music_sync > 750)
		{
			if (vball_phase > VBALL_PHASE_INTRO_FADE)
				drawVectorBalls(RSE_COUNTER(angle), RSE_COUNTER(sec_angle));

			if ((angle & 0xFE) == 0)
				sec_angle_step += 25;

			sec_angle += sec_angle_step;
			if (sec_angle_step >= 1)
				sec_angle_step--;			
		}
		
		if (music_sync > 790)
			updateScrollText();

		u8 music_checkpoint = 0;

		switch(vball_phase)
		{
			case VBALL_PHASE_INTRO_SCROLL:
				SPR_setPosition(prev_sprites[0], (320 - 240) >> 1, 72 - (easing_table[vball_timer << 4] / 25));
				SPR_update(prev_sprites, 1);

				VDP_setVerticalScroll(PLAN_B, (easing_table[vball_timer << 4] >> 4) - 64);
				vball_timer++;

				switch(music_checkpoint)
				{
					case 0:
						if (music_sync >= 585) // (vball_timer > RSE_FRAMES(63))
						{
							VDP_fadePalTo(PAL1, sky.palette->data, RSE_FRAMES(32), TRUE);
							music_checkpoint++;
						}
						break;
				}

				if (vball_timer > RSE_FRAMES(63))
				{
					VDP_setVerticalScroll(PLAN_B, 0);
					VDP_setScrollingMode(HSCROLL_LINE, VSCROLL_2TILE);
					VDP_fadePalTo(PAL1, sky.palette->data, RSE_FRAMES(32), TRUE);
					vball_timer = 0;
					vball_phase++;					
				}			
				break;

			case VBALL_PHASE_INTRO_LOGO_FADEOUT:

				if (music_sync > 650 && music_sync < 750)
				{
					if (music_sync < 700)
					{
						sine_ramp++;
						if (sine_ramp > 512)
							sine_ramp = 512;
					}
					else
					{
						sine_ramp--;
						if (sine_ramp < 0)
							sine_ramp = 0;		
					}

					for(loop = 0; loop < 256; loop++)
						ramped_sine_scroll[loop] = (sine_scroll[((angle << 2) + loop) & 0xFE] * ((sine_ramp >> 2) & 0xF)) >> 4;

					VDP_setHorizontalScrollLine(PLAN_B, 0, ramped_sine_scroll, 224, TRUE);
				}
				else
				if (music_sync > 750)
				{
					VDP_fadePalTo(PAL2, palette_white_bg, 16, TRUE);
					vball_phase++;
				}
				break;

			case VBALL_PHASE_INTRO_FADE:
				vball_timer++;
				if (!VDP_isDoingFade())
				{
					clearTitleSprite();		
					initVectorBallsSprites();
					vball_timer = 0;
					vball_phase++;
				}			
				break;

			case VBALL_PHASE_BEGIN:
				VDP_interruptFade();
				VDP_setScrollingMode(HSCROLL_PLANE, VSCROLL_2TILE);
				VDP_fadePalTo(PAL2, palette_white, RSE_FRAMES(8), TRUE);
				vball_timer = 0;
				vball_phase++;
				break;

			case VBALL_PHASE_FADETOWHITE:
				vball_timer++;
				if (vball_timer > RSE_FRAMES(8))
				{
					if (object_idx & 0x1)
						VDP_fadePalTo(PAL2, ball_metal.palette->data, RSE_FRAMES(16), TRUE);
					else
						VDP_fadePalTo(PAL2, masiaka_title.palette->data, RSE_FRAMES(16), TRUE);
					vball_timer = 0;
					vball_phase++;
				}
				break;				

			case VBALL_PHASE_FADEIN:
				vball_timer++;
				if (vball_timer > RSE_FRAMES(16))
				{
					vball_timer = 0;
					vball_phase++;
				}
				break;

			case VBALL_PHASE_RUN:
				vball_timer++;
				if (vball_timer > RSE_FRAMES(62 * 10))
				{
					VDP_fadeOut(32, 32 + 15, RSE_FRAMES(32), TRUE);
					vball_timer = 0;
					vball_phase++;
				}
				break;

			case VBALL_PHASE_FADEOUT:
				vball_timer++;
				if (vball_timer > RSE_FRAMES(32))
				{
					vball_timer = 0;
					vball_phase++;
				}
				break;

			case VBALL_NEXT_OBJECT:
				object_idx++;
				zsort_switch = 0;

				if (object_idx >= MAX_VBALL_OBJECTS)
				{
					object_idx = 0;
					vball_phase = VBALL_PHASE_QUIT;
				}
				else
					vball_phase = VBALL_PHASE_BEGIN;

				switch(object_idx)
				{

					case 0:
						ball_count = grid_cube_small_VTX_COUNT;
						vector_ball_array = (Vect3D_f16 *)vb_grid_cube_small_vertex_pos;
						break;

					case 1:
						ball_count = sword_VTX_COUNT;
						vector_ball_array = (Vect3D_f16 *)vb_sword_vertex_pos;
						break;

					case 2:
						ball_count = pyramid_VTX_COUNT;
						vector_ball_array = (Vect3D_f16 *)vb_pyramid_vertex_pos;
						break;						
				}
		}
	}

	SPR_clear();
	SPR_end();
	vramIndex = vramIndex_mem;
	VDP_waitVSync();
	updateScrollText();
	DMA_flushQueue();
}