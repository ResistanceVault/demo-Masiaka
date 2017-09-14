#include <genesis.h>
#include <gfx.h>
#include "writer.h"
#include "transition_helper.h"
#include "quicksort.h"

#define SCREEN_HEIGHT 224

extern u16 vramIndex;
// extern u16 fontIndex;
extern u8 framerate;

NOINLINE void displayBarbPictureFX(void)
{
	u16 fx_phase, scroll_A_y, i, fx_len;
	u32 music_sync = 0;
	u16 pal_fx0[16];
	u8 f_step = 0, fading = FALSE;

	RSE_turn_screen_to_black();

	VDP_waitVSync(); // start on a clean VSync
	VDP_setEnable(0);

	RSE_changeResolution(320);

	/* Set a larger tileplan to be able to scroll */
	VDP_setPlanSize(64, 32);
	VDP_setHilightShadow(0);

	vramIndex = TILE_USERINDEX;

	/* Draw the background */
	VDP_drawImageEx(PLAN_B, &barb_pic_2_back, TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, vramIndex), 0, 0, FALSE, TRUE);
	vramIndex += barb_pic_2_back.tileset->numTile;

	/* Draw the foreground */
	VDP_drawImageEx(PLAN_A, &barb_pic_2_front, TILE_ATTR_FULL(PAL1, FALSE, FALSE, FALSE, vramIndex), 0, 0, FALSE, TRUE);
	vramIndex += barb_pic_2_front.tileset->numTile;

	VDP_setScrollingMode(HSCROLL_PLANE, VSCROLL_PLANE);

	VDP_setVerticalScroll(PLAN_A, 0);
	VDP_setVerticalScroll(PLAN_B, 0);

	DMA_flushQueue();
	VDP_setEnable(1);

	/* prepare a specific palette for a flash fx */
	for(i = 0; i < 16; i++)
		pal_fx0[i] = RSE_colMul(barb_pic_2_front.palette->data[i], RSE_colMul(barb_pic_2_front.palette->data[i], RSE_colMul(barb_pic_2_front.palette->data[i], barb_pic_2_front.palette->data[i])));


	fx_phase = 0;
	fx_len = framerate == 60 ? 1040 : 750;
	while(fx_phase < fx_len)
	{
		VDP_waitVSync();

		music_sync = music_getElapsed();
		// KLog_U2("barb_picture.c, music_sync = ", music_sync, ", fx_phase = ", fx_phase);	

		if (fading)
			fading = VDP_doStepFading(0);

		switch(f_step)
		{
			case 0:
				if (!fading && fx_phase > RSE_FRAMES(58))
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
				if (!fading && fx_phase > RSE_FRAMES(110))
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
					VDP_fade(16, 31, palette_white, barb_pic_2_front.palette->data, RSE_FRAMES(16), TRUE);
					VDP_interruptFade();
					fading = 1;
					f_step++;		
				}
				break;

			case 4:
				if (!fading && fx_phase > RSE_FRAMES(225))
				{
					// KLog_U1("-> f_step = ", f_step);
					VDP_fade(0, 15, palette_black, barb_pic_2_back.palette->data, RSE_FRAMES(16), TRUE);
					VDP_interruptFade();
					fading = 1;						
					f_step++;
				}
				break;
		}						

		scroll_A_y = (easing_table[min(EASING_TABLE_LEN - 1, max(0, fx_phase - RSE_FRAMES(240)) << 3)] * (256 - 224)) >> 10; // 256 - 224

		VDP_setVerticalScroll(PLAN_A, scroll_A_y);
		VDP_setVerticalScroll(PLAN_B, scroll_A_y);
		fx_phase++;
	}

	u16 cur_color = 0, col_idx;
	u16 pal[32];
	struct QSORT_ENTRY pal_sort[32];

	memcpyU16(&pal[0], barb_pic_2_back.palette->data, 16);
	memcpyU16(&pal[16], barb_pic_2_front.palette->data, 16);
	
	for (i = 0; i < 32 ; ++i)
	{
		pal_sort[i].index = i;
		pal_sort[i].value =
				fix16Mul(FIX16(0.299), FIX16(pal[i] & 0xf)) +
				fix16Mul(FIX16(0.587), FIX16((pal[i] >> 4) & 0xf)) +
				fix16Mul(FIX16(0.114), FIX16((pal[i] >> 8) & 0xf));
	}

	QuickSort(32, pal_sort);
	
	fading = 0;
	while(TRUE)
	{
		VDP_waitVSync();
		
		if (!fading)
		{
			if (cur_color != 31)
				cur_color++;
			else
				break;
			
			col_idx = pal_sort[cur_color].index;
			VDP_initFading(col_idx, col_idx, &pal[col_idx], &palette_black[0], 4, FALSE);
			fading = 1;
		}
		else
		{
			fading = VDP_doStepFading(FALSE);
		}
	}			
			
	RSE_clearAll();
}