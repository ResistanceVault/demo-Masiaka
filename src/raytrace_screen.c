#include "genesis.h"
#include <gfx.h>
#include "transition_helper.h"
#include "res/gfx.h"
#include "music.h"

#define SCREEN_HEIGHT 224

#define RT_WIDTH 320
#define RT_HEIGHT 176
#define RT_NUM_FRAMES 16
#define RT_START_TILE 32

#define FX_LEN RSE_FRAMES(60 * 16)
#define INIT_FADE RSE_FRAMES(16)
#define END_FADE RSE_FRAMES(32)

#define RT_SIZE (RT_WIDTH * RT_HEIGHT / 2) // in bytes

extern u16 vramIndex;
// extern u16 fontIndex;
extern u8 framerate;

static vu8 readyToUpload;

static void preparePlan(u8 palette)
{
	u16 i, j, k, pw, sz;
	u16* buf;
	
	pw = VDP_getPlanWidth();
	sz = pw * VDP_getPlanHeight();
	
	buf = (u16*)MEM_alloc(sz << 1);
	memsetU16(buf, 0, sz);
	
	k = RT_START_TILE;
	for (j = 0; j < RT_HEIGHT / 8; ++j) {
		for (i = 0; i < RT_WIDTH / 8; ++i) {
			buf[pw * j + i] = TILE_ATTR_FULL(palette, 0, 0, 0, k);
			buf[pw * j + i + pw * 32] = TILE_ATTR_FULL(palette, 0, 0, 0, k + RT_SIZE / TILE_SIZE);
			++k;
		}
	}
	
	VDP_setTileMapData(VDP_getBPlanAddress(), buf, 0, sz, TRUE);
	
	MEM_free(buf);
}
	
static void hInt(void) {
	readyToUpload = 1;
}

NOINLINE void raytraceFX(void)
{
	_voidCallback *hIntSave;
	u16 aAddrSave = VDP_getAPlanAddress();
	u16 bAddrSave = VDP_getBPlanAddress();
	u16 sAddrSave = VDP_getSpriteListAddress();
	u16 hAddrSave = VDP_getHScrollTableAddress();
	u8 i;
	u16 pal_fx0[16], pal_fx1[16], pal_fx2[16];
	u32 music_sync;

	VDP_waitVSync(); // start on a clean VSync
	VDP_setEnable(0);
	
	RSE_changeResolution(320);

	VDP_setScrollingMode(HSCROLL_PLANE, VSCROLL_PLANE);
	VDP_setPlanSize(64, 64);
	maps_addr = 0x0000; // prevent SGDK from loading font

	VDP_setAPlanAddress(0xe000);
	VDP_setBPlanAddress(0xe000);
	VDP_setSpriteListAddress(0xfe00);
	VDP_setHScrollTableAddress(0xfc00);

	VDP_clearPlan(PLAN_A, TRUE);
	VDP_clearPlan(PLAN_B, TRUE);
	VDP_clearSprites();

	preparePlan(PAL1);

	VDP_setHIntCounter((SCREEN_HEIGHT + RT_HEIGHT) >> 1);
	hIntSave = internalHIntCB;
	internalHIntCB = hInt;
	VDP_setHInterrupt(1);
	
	VDP_setHorizontalScroll(PLAN_A, 0);
	VDP_setHorizontalScroll(PLAN_B, 0);
	
	VDP_setEnable(1);

	for(i = 0; i < 16; i++)
	{
		pal_fx2[i] = RSE_colMul(rt_pal.data[i], rt_pal.data[i]);
		pal_fx1[i] = RSE_colMul(pal_fx2[i], rt_pal.data[i]);
		pal_fx0[i] = RSE_colMul(pal_fx1[i], rt_pal.data[i]);
	}

// music_sync = music_getElapsed();
// // KLog_U1("raytrace_screen.c, music_sync = ", music_sync);	

// RSE_turn_screen_to_color(0xF0F);
	while(music_sync < 7070 + 25)
	{
		VDP_waitVSync();
		music_sync = music_getElapsed();
	}
// RSE_turn_screen_to_color(0x000);

	u16 vcount = 0;
	u8 f_step = 0;
	u16 fxLen = FX_LEN;
	u16 fadeoutStart = FX_LEN - END_FADE;

	// music_sync = music_getElapsed();
	// // KLog_U1("raytrace_screen.c, music_sync = ", music_sync);

	VDP_initFading(16, 31, palette_black, bw_pal.data, INIT_FADE - 2, FALSE);
	u8 fading = 1;

	while(music_sync < 7790 - 100 + 50 + 25)
	{
		music_sync = music_getElapsed();

		// if (JOY_readJoypad(0))
		// KLog_U1("music_sync = ", music_sync);

		if (fading)
			fading = VDP_doStepFading(0);

		// music sync : 7070
		// 7300

		switch(f_step)
		{
			case 0:
				if (!fading && music_sync > 7135)
				{
					// KLog_U1("-> f_step = ", f_step);
					VDP_initFading(16, 31, bw_pal.data, palette_white, 4, FALSE);
					fading = 1;						
					f_step++;
				}
				break;

			case 1:
				if (!fading)
				{
					// KLog_U1("-> f_step = ", f_step);
					VDP_initFading(16, 31, palette_white, bw_pal.data, 4, FALSE);
					fading = 1;
					f_step++;		
				}
				break;

			case 2:
				if (!fading && music_sync > 7235) // 7160
				{
					// KLog_U1("-> f_step = ", f_step);
					VDP_initFading(16, 31, bw_pal.data, palette_white, 4, FALSE);
					fading = 1;						
					f_step++;
				}
				break;

			case 3:
				if (!fading)
				{
					// KLog_U1("-> f_step = ", f_step);
					VDP_initFading(16, 31, palette_white, pal_fx0, INIT_FADE, FALSE);
					fading = 1;
					f_step++;		
				}
				break;

			case 4:
				if (!fading && music_sync > 7330) // 7210
				{
					// KLog_U1("-> f_step = ", f_step);
					VDP_initFading(16, 31, pal_fx0, palette_white, 4, FALSE);
					fading = 1;						
					f_step++;
				}
				break;

			case 5:
				if (!fading)
				{
					// KLog_U1("-> f_step = ", f_step);
					VDP_initFading(16, 31, palette_white, pal_fx1, INIT_FADE, FALSE);
					fading = 1;
					f_step++;	
				}
				break;

			case 6:
				if (!fading && music_sync >  7380) // 7265
				{
					// KLog_U1("-> f_step = ", f_step);
					VDP_initFading(16, 31, pal_fx1, palette_white, 4, FALSE);
					fading = 1;						
					f_step++;
				}
				break;

			case 7:
				if (!fading)
				{
					// KLog_U1("-> f_step = ", f_step);
					VDP_initFading(16, 31, palette_white, pal_fx2 /*rt_pal.data*/, INIT_FADE, FALSE);
					fading = 1;
					f_step++;		
				}
				break;

			case 8:
				if (!fading && music_sync > 7425) // 7310
				{
					// KLog_U1("-> f_step = ", f_step);
					VDP_initFading(16, 31, pal_fx2, palette_white, 4, FALSE);
					fading = 1;						
					f_step++;
				}
				break;

			case 9:
				if (!fading)
				{
					// KLog_U1("-> f_step = ", f_step);
					VDP_initFading(16, 31, palette_white, rt_pal.data, INIT_FADE, FALSE);
					fading = 1;
					f_step++;		
				}
				break;				

			case 10:
				if (!fading && music_sync > 7790 - 100 + 25)
				{
					// KLog_U1("-> f_step = ", f_step);
					VDP_initFading(16, 31, rt_pal.data, palette_black, END_FADE * 2, FALSE);
					fading = 1;
				}			
				break;

		}
		
		readyToUpload = 0;
		while (!readyToUpload);
		VDP_setEnable(0);
		
		DMA_doDma(DMA_VRAM,
				(u32) rt_anim.tiles + (vcount % (RT_NUM_FRAMES * 2)) * (RT_SIZE / 2),
				RT_START_TILE * TILE_SIZE + (RT_SIZE / 2) * (vcount & 3),
				RT_SIZE / 4, 2);
		
		VDP_setEnable(1);

		s16 vscroll = (vcount & 2 ? 0 : 256) + ((RT_HEIGHT - SCREEN_HEIGHT) >> 1);
		VDP_setVerticalScroll(PLAN_A, vscroll);
		VDP_setVerticalScroll(PLAN_B, vscroll);
		
		vcount++;
	}
	VDP_waitFadeCompletion();

	VDP_setAPlanAddress(aAddrSave);
	VDP_setHScrollTableAddress(hAddrSave);
	VDP_setBPlanAddress(bAddrSave);
	VDP_setSpriteListAddress(sAddrSave);

	internalHIntCB = hIntSave;
	RSE_clearAll();
}
