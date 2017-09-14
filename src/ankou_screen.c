#include <genesis.h>
#include <gfx.h>
#include "transition_helper.h"
#include "music.h"

extern u16 vramIndex;
// extern u16 fontIndex;
extern u8 framerate;

NOINLINE void ankouScreenFX(void){
	u16 vcount, i;
	u32 music_sync = 0;
	u16 pal_fx0[16], pal_fx1[16];

	VDP_setEnable(0);

	RSE_turn_screen_to_black();

	RSE_changeResolution(320);

	// SYS_disableInts();

	VDP_clearPlan(PLAN_A, TRUE);
	VDP_clearPlan(PLAN_B, TRUE);
	/* Set a larger tileplan to be able to scroll */
	VDP_setPlanSize(64, 32);

	/* Draw the background */	

	VDP_setScrollingMode(HSCROLL_PLANE, VSCROLL_PLANE);
	VDP_setVerticalScroll(PLAN_B, 0);
	VDP_setVerticalScroll(PLAN_A, 0);
	VDP_setHorizontalScroll(PLAN_B, 0);
	VDP_setHorizontalScroll(PLAN_A, 0);

	vramIndex = TILE_USERINDEX;
	VDP_waitDMACompletion();

	/* Pic 0 */
	VDP_drawImageEx(PLAN_B, &exocet_ankou_front_0, TILE_ATTR_FULL(PAL1, TRUE, FALSE, FALSE, vramIndex), 0, 0,  FALSE, TRUE);
	vramIndex += exocet_ankou_front_0.tileset->numTile;

	VDP_drawImageEx(PLAN_B, &exocet_ankou_front_1, TILE_ATTR_FULL(PAL1, TRUE, FALSE, FALSE, vramIndex), 80 >> 3, 0, FALSE, TRUE);
	vramIndex += exocet_ankou_front_1.tileset->numTile;

	VDP_drawImageEx(PLAN_B, &exocet_ankou_front_2, TILE_ATTR_FULL(PAL1, TRUE, FALSE, FALSE, vramIndex), 160 >> 3, 0, FALSE, TRUE);
	vramIndex += exocet_ankou_front_2.tileset->numTile;

	VDP_drawImageEx(PLAN_B, &exocet_ankou_front_3, TILE_ATTR_FULL(PAL1, TRUE, FALSE, FALSE, vramIndex), 240 >> 3, 0, FALSE, TRUE);
	vramIndex += exocet_ankou_front_3.tileset->numTile;

	VDP_setEnable(1);

	// VDP_fadePalTo(PAL1, exocet_ankou_darken.palette->data, RSE_FRAMES(32), TRUE);

	VDP_drawImageEx(PLAN_A, &exocet_ankou_back_0, TILE_ATTR_FULL(PAL0, TRUE, FALSE, FALSE, vramIndex), 0, 0, FALSE, TRUE);
	vramIndex += exocet_ankou_back_0.tileset->numTile;

	VDP_drawImageEx(PLAN_A, &exocet_ankou_back_1, TILE_ATTR_FULL(PAL0, TRUE, FALSE, FALSE, vramIndex), 80 >> 3, 0, FALSE, TRUE);
	vramIndex += exocet_ankou_back_1.tileset->numTile;

	VDP_drawImageEx(PLAN_A, &exocet_ankou_back_2, TILE_ATTR_FULL(PAL0, TRUE, FALSE, FALSE, vramIndex), 160 >> 3, 0, FALSE, TRUE);
	vramIndex += exocet_ankou_back_2.tileset->numTile;

	VDP_drawImageEx(PLAN_A, &exocet_ankou_back_3, TILE_ATTR_FULL(PAL0, TRUE, FALSE, FALSE, vramIndex), 240 >> 3, 0, FALSE, TRUE);
	vramIndex += exocet_ankou_back_3.tileset->numTile;

	/* prepare a specific palette for a flash fx */
	for(i = 0; i < 16; i++)
	{
		pal_fx0[i] = RSE_colMul(exocet_ankou_front_0.palette->data[i], RSE_colMul(exocet_ankou_front_0.palette->data[i], RSE_colMul(exocet_ankou_front_0.palette->data[i], exocet_ankou_front_0.palette->data[i])));
		pal_fx1[i] = RSE_colMul(exocet_ankou_back_3.palette->data[i], RSE_colMul(exocet_ankou_back_3.palette->data[i], exocet_ankou_back_3.palette->data[i]));
	}

	while(music_sync < 7750)
	{
		VDP_waitVSync();
		music_sync = music_getElapsed();
	}

	music_sync = music_getElapsed();
	// KLog_U1("music_sync = ", music_sync);

	// RSE_pause(RSE_FRAMES(16));
	// VDP_fadePalTo(PAL1, exocet_ankou_front_0.palette->data, RSE_FRAMES(24), TRUE);
	// RSE_pause(RSE_FRAMES(24));
	// VDP_fadePalTo(PAL0, exocet_ankou_back_0.palette->data, RSE_FRAMES(16), TRUE);
	// RSE_pause(RSE_FRAMES(16));	

	u8 f_step = 0, fading = FALSE;

	vcount = 0;
	while(music_sync < 8040) // vcount < RSE_FRAMES(60 * 5))
	{
		VDP_waitVSync();

		if (fading)
			fading = VDP_doStepFading(0);		

		music_sync = music_getElapsed();
		// KLog_U1("music_sync = ", music_sync);

		switch(f_step)
		{
			case 0:
				if (!fading && music_sync > 7750 + 50)
				{
					// KLog_U1("-> f_step = ", f_step);
					VDP_fade(16, 31, palette_black, palette_white, 4, TRUE);
					VDP_interruptFade();
					fading = 1;						
					f_step++;
				}
				break;

			case 1:
				if (!fading)
				{
					// KLog_U1("-> f_step = ", f_step);
					VDP_fade(16, 31, palette_white, pal_fx0, 8, TRUE);
					VDP_interruptFade();
					fading = 1;
					f_step++;		
				}
				break;

			case 2:
				if (!fading && music_sync > 7750 + 100)
				{
					// KLog_U1("-> f_step = ", f_step);
					VDP_fade(16, 31, pal_fx0, palette_white, 4, TRUE);
					VDP_interruptFade();
					fading = 1;						
					f_step++;
				}
				break;

			case 3:
				if (!fading)
				{
					// KLog_U1("-> f_step = ", f_step);
					VDP_fade(16, 31, palette_white, exocet_ankou_front_0.palette->data, RSE_FRAMES(16), TRUE);
					VDP_interruptFade();
					fading = 1;
					f_step++;		
				}
				break;

			case 4:
				if (!fading && music_sync > 7750 + 150)
				{
					// KLog_U1("-> f_step = ", f_step);
					VDP_fade(0, 15, palette_black, pal_fx1, 4, TRUE);
					VDP_interruptFade();
					fading = 1;						
					f_step++;
				}
				break;

			case 5:
				if (!fading)
				{
					// KLog_U1("-> f_step = ", f_step);
					VDP_fade(0, 15, pal_fx1, exocet_ankou_back_0.palette->data, RSE_FRAMES(16), TRUE);
					VDP_interruptFade();
					fading = 1;
					f_step++;		
				}
				break;
		}		

		vcount++;
	}

	VDP_fadeOut(1, 63, RSE_FRAMES(16), FALSE);

	RSE_clearAll();
}