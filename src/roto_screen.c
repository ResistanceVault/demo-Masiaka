#include "genesis.h"
#include <gfx.h>
#include "transition_helper.h"
#include "rotozoomfx.h"
#include "fb.h"
#include "res/gfx.h"
#include "easing_table_fp.h"
#include "music.h"

#define SCREEN_HEIGHT 224
#define BOTTOM_WINDOW_HEIGHT 16

#define RZ_X_OFFSET 44
#define RZ_Y_OFFSET -28
#define PILLAR_Y_OFFSET -16

#define RZ_INIT_ANGLE 0
#define RZ_INIT_ZOOM FIX16(1.5)
#define RZ_TABLES_LEN 1024

#define WATER_STEP 3

#define WATER_FADE_START RSE_FRAMES(20)
#define FX_LEN RSE_FRAMES(550)
#define TARGET_FX_LEN RSE_FRAMES(60 * 8)

#define INIT_FADE RSE_FRAMES(8)
#define WATER_FADE RSE_FRAMES(8)
#define END_FADE RSE_FRAMES(8)

extern void FB_hInt(void);

extern u16 vramIndex;
// extern u16 fontIndex;
extern u8 framerate;

const u32 sync_table[] = {8085, 8270, 8435, 8460, 8625, 8650, 8815, 8840};

static void preparePlan(u8 palette)
{
	u16 i, j, pw, sz;
	u16* buf;
	
	pw = VDP_getPlanWidth();
	sz = pw * (RZ_HEIGHT / 8 + 1);
	
	buf = (u16*)MEM_alloc(sz << 1);
	memsetU16(buf, 0, sz);
	
	for (j = 0; j < RZ_HEIGHT / 8; ++j) {
		for (i = 0; i < RZ_WIDTH / 8; ++i) {
			buf[pw * (RZ_HEIGHT / 8 - j - 1) + i] = TILE_ATTR_FULL(palette, 0, 1, 0, FB_TILES_OFFSET + (RZ_HEIGHT / 8) * i + j);
		}
	}
	
	for (i = 0; i < RZ_WIDTH / 8; ++i) {
		buf[pw * (RZ_HEIGHT / 8) + i] = TILE_ATTR_FULL(palette, 0, 0, 0, FB_TILES_OFFSET + (RZ_HEIGHT / 8) * i);
	}
	
	VDP_setTileMapData(FB_getPlanAddress(), buf, pw * 32, sz, TRUE);
	
	MEM_free(buf);
}
	
static s16 *water;
static s16 *water_lst;
static s16 *water_ptr;

static void hInt(void)
{
	s16 w = *water_ptr++;
	
	VDP_setVerticalScroll(PLAN_B, w);

	if(water_ptr > water_lst)
	{
		VDP_setHIntCounter(0xf0);
		internalHIntCB = FB_hInt;
	} 
}

static void hIntPrep(void)
{
	VDP_setHIntCounter(WATER_STEP - 1);
	internalHIntCB = hInt;
}

static void vInt(void) {
	static u16 vc = 0;

	VDP_setVerticalScroll(PLAN_B, RZ_Y_OFFSET);
	internalHIntCB = hIntPrep;
	VDP_setHIntCounter((RZ_HEIGHT - RZ_Y_OFFSET) / 2 - 1);

	water_ptr = &water[(vc & 0x1f) << 5];
	water_lst = water_ptr + RZ_HEIGHT / 2 / WATER_STEP;
	vc++;
}

NOINLINE void rotoFX(void)
{
	u16 pal[64];
	u8 bgSave = VDP_getBackgroundColor();
	u16 windowSave = VDP_getWindowAddress();
	u32 music_sync = 0;
	fix16 *angles, *zooms;
	fix16 ainc;
	u8 fading = 1;
	
	vramIndex = TILE_USERINDEX;

	memcpyU16(&pal[0], roto_col.palette->data, 16);
	memcpyU16(&pal[16], roto_tex.palette->data, 16);
	for (s16 i = 32; i < 48; ++i)
		pal[i] = roto_tex_water_pal.palette->data[i - 32];
	
	VDP_waitVSync(); // start on a clean VSync
	VDP_setEnable(0);
	
	RSE_changeResolution(256);

	VDP_setPlanSize(32, 64);
	VDP_drawImageEx(PLAN_A, &roto_col, TILE_ATTR_FULL(PAL0, TRUE, FALSE, FALSE, vramIndex), 1, 0, FALSE, TRUE);

	for(s16 i = 0; i < roto_col.map->h; ++i)
	{
		for(s16 j = 0; j < roto_col.map->w; ++j)
		{
			u16 itm = (roto_col.map->tilemap[roto_col.map->w * i + j] + vramIndex)
						| TILE_ATTR_HFLIP_MASK | TILE_ATTR_PRIORITY_MASK;
			VDP_setTileMapXY(PLAN_A, itm, 30 - j, i);
		}
	}

	vramIndex += roto_col.tileset->numTile;

	rzfx_init(PLAN_B, PAL1, roto_tex.image);
	preparePlan(PAL2);
	VDP_setHorizontalScroll(PLAN_B, RZ_X_OFFSET);
	VDP_setVerticalScroll(PLAN_A, PILLAR_Y_OFFSET);
	SYS_setVIntCallback(vInt);
	VDP_setBackgroundColor(15);
	
	VDP_setWindowAddress(0x4000);
	VDP_setWindowHPos(FALSE, 0);
	VDP_setWindowVPos(TRUE, (SCREEN_HEIGHT - BOTTOM_WINDOW_HEIGHT) / 8);
	VDP_clearPlan(PLAN_WINDOW, TRUE);
	
	angles = MEM_alloc(RZ_TABLES_LEN * sizeof(fix16));
	zooms = MEM_alloc(RZ_TABLES_LEN * sizeof(fix16));
	water = MEM_alloc(RZ_TABLES_LEN * sizeof(fix16));

	ainc = 0;
	for(s16 i = 0; i < RZ_TABLES_LEN; ++i)
	{
		u16 vcountf = i * 60 / framerate;
		
		if (vcountf >= 256 && vcountf < 320)
		{
			ainc = (vcountf - 256) * 384;
			vcountf = 256;
		}
		else if (vcountf >= 320)
		{
			vcountf -=64;
		}
		
		angles[i] = RZ_INIT_ANGLE + fix16ToRoundedInt(fix16Mul(ainc + (easing_fix16[vcountf & (EASING_TABLE_LEN_FP - 1)] << 7), FIX16(0.859)));
		
		if (vcountf >= EASING_TABLE_LEN_FP / 2)
		{
			zooms[i] = RZ_INIT_ZOOM - (easing_fix16[EASING_TABLE_LEN_FP - 1 - (((vcountf << 1) & (EASING_TABLE_LEN_FP - 1)))] / 3);
		}
		else
		{
			zooms[i] = RZ_INIT_ZOOM - (easing_fix16[vcountf << 1] / 3);
		}
		
		u16 posWater = i & 0x1f;

		if (posWater < RZ_HEIGHT / 2 / WATER_STEP)
		{
			s16 v = posWater * WATER_STEP + ((cosFix16(i + 4096 / (1 + posWater)) * (2 + posWater)) >> 7);
			water[i] = RZ_Y_OFFSET - RZ_HEIGHT + 256 + abs(v);
		}
		else
		{
			water[i] = 504;
		}
	}
	
	VDP_setEnable(1);
	
	VDP_setHInterrupt(1);

	u16 vcount = 0;
	u16 waterFadeStart = WATER_FADE_START;
	u16 fadeoutStart = FX_LEN - END_FADE;
	u16 fxLen = FX_LEN;

	while(music_sync < 8080)
	{
		VDP_waitVSync();
		music_sync = music_getElapsed();
	}
	
	VDP_initFading(0, 31, palette_black, pal, INIT_FADE, TRUE);

	while(vcount < fxLen)
	{
		// music_sync = music_getElapsed();
		// KLog_U2("music_sync = ", music_sync, "vcount = ", vcount);

		if (fading)
			fading = VDP_doStepFading(0);
		
		rzfx_loop(RZ_TEX_SIZE / 4, RZ_TEX_SIZE / 4,	angles[vcount], zooms[vcount]);
		
		if (vcount == waterFadeStart)
		{
			VDP_initFading(32, 47, palette_black, &pal[32], WATER_FADE, FALSE);
			fading = 1;
		}
		else if (vcount == fadeoutStart)
		{
//			music_sync = music_getElapsed();
//			KLog_U2("music_sync = ", music_sync, "vcount = ", vcount);			
			VDP_initFading(0, 47, pal, palette_black, END_FADE, FALSE);
			fading = 1;
		}
		
		vcount++;
	}
	VDP_waitFadeCompletion();
	
//	music_sync = music_getElapsed();
//	KLog_U2("music_sync = ", music_sync, "vcount = ", vcount);	

	MEM_free(water);
	MEM_free(zooms);
	MEM_free(angles);
	FB_close();

	VDP_setWindowAddress(windowSave);
	VDP_setBackgroundColor(bgSave);

	RSE_clearAll();
}