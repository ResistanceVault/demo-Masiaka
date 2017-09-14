#include <genesis.h>
#include <gfx.h>
#include "writer.h"
#include "transition_helper.h"
#include "demo_strings.h"
#include "page_writer.h"
#include "logos_vertices.h"
#include "music.h"

#define FX_ENABLE_WRITER
#define FX_ENABLE_SPRITES

#define MAX_SPRITE	MAX_LOGO_VLEN
#define MAX_FADE_SPRITE 32

#define BG_TINT_0 0x224
#define BG_TINT_1 0x642
#define BG_TINT_2 0x620
#define BG_TINT_3 0x420

extern u16 vramIndex;
// extern u16 fontIndex;
extern u8 framerate;

static u16 palette_bg_0_start[16], palette_bg_0[16], palette_bg_1[16], palette_bg_2[16], palette_bg_3[16];
static 	Sprite *sprites[MAX_SPRITE];

static s16 shield_hscroll[256];
static s16 scroll_tile_x[32], scroll_dir[32], wipe_tile_x[32];
static u16 const *logo_xy;

static u16 tileIndexes[64];

NOINLINE void shieldAnimFX(void)
{
	u8 spr_max, logo_idx;
	u16 i, j, k, l, vcount, vcount2;
	s16 si, sj;
	u8 logo_on_top, should_exit, logos_done;
	u8 letter_state;
	s16 letter_transition;
	u32 music_sync;
	Object sb_objects[MAX_SPRITE];

	VDP_setEnable(0);

	RSE_changeResolution(320);
	// VDP_setScreenWidth320();

	vramIndex = TILE_USERINDEX;
	RSE_reuploadSystemTiles();
	/*fontIndex = */RSE_pgwriterSetup();
	
	/* Set a larger tileplan to be able to scroll */
	VDP_setPlanSize(128, 32);
	VDP_setHilightShadow(1); 
	VDP_setScrollingMode(HSCROLL_TILE, VSCROLL_PLANE);

	// VDP_setPalette(PAL0, shield_anim_0.palette->data);

	// vramIndex = fontIndex;

	/* Draw the background */
	for(i = 0; i < 4; i++)
		VDP_drawImageEx(PLAN_B, &shield_anim_0, TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, vramIndex), 0, i << 3 , FALSE, TRUE);

	vramIndex += shield_anim_0.tileset->numTile;

	for(i = 0; i < 4; i++)
		VDP_drawImageEx(PLAN_B, &shield_anim_1, TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, vramIndex), 512 >> 3, i << 3 , FALSE, TRUE);

	vramIndex += shield_anim_1.tileset->numTile;

	for(i = 0; i < 16; i++)
		VDP_drawImageEx(PLAN_A, &transition_pattern, TILE_ATTR_FULL(PAL1, TRUE, FALSE, FALSE, vramIndex), 0, i << 1, FALSE, FALSE);

	vramIndex += transition_pattern.tileset->numTile;

#ifdef FX_ENABLE_SPRITES
	SPR_init(MAX_SPRITE,0,0);

    for(i = 0; i < ball_square.animations[0]->numFrame; i++)
    {
        TileSet* tileset = ball_square.animations[0]->frames[i]->tileset;

        VDP_loadTileSet(tileset, vramIndex, TRUE);
        tileIndexes[i] = vramIndex;
        vramIndex += tileset->numTile;
    }

	/* sprites */
	for(i = 0; i < MAX_SPRITE * 2; i+=2)
	{
		sprites[i >> 1] = SPR_addSpriteEx(&ball_square, -80, -80, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, tileIndexes[0]), 0, !SPR_FLAG_AUTO_VISIBILITY | !SPR_FLAG_AUTO_VRAM_ALLOC | SPR_FLAG_AUTO_SPRITE_ALLOC | !SPR_FLAG_AUTO_TILE_UPLOAD);
		SPR_setAlwaysVisible(sprites[i >> 1], TRUE);
		sprites[i >> 1]->data = (u32) &sb_objects[i >> 1];
	}

	SPR_update(MAX_SPRITE);
#endif

	DMA_flushQueue();
	VDP_setEnable(1);

	VDP_setPaletteColor(17, 0x000);

	for (i = 0; i < 32; i++)
		scroll_tile_x[i] = 0;

	for (i = 0; i < 32; i++)
		wipe_tile_x[i] = 0;

	VDP_waitVSync();

	for (i = 0; i < 4; i++)
	{ 
		VDP_waitVSync();
		for(j = 0; j < 8; j++)
		{
			shield_hscroll[i * 8 + j] = i * 4 * 64;
			if (i & 0x1)
				shield_hscroll[i * 8 + j] += 32;
			if (i && 0x1)
				scroll_dir[i * 8 + j] = 64;
			else
				scroll_dir[i * 8 + j] = -64;
		}
	}

	for(i = 0; i < 16; i++)
	{
		palette_bg_0_start[i] = shield_anim_0.palette->data[i];
		palette_bg_0[i] = shield_pal_1.palette->data[i];
		palette_bg_1[i] = shield_pal_2.palette->data[i];
		palette_bg_2[i] = shield_pal_3.palette->data[i];
		palette_bg_3[i] = shield_pal_4.palette->data[i];
	}

	palette_bg_0_start[0] = flames_0.palette->data[2];
	palette_bg_0[0] = flames_0.palette->data[2]; 
	palette_bg_1[0] = flames_0.palette->data[2]; 
	palette_bg_2[0] = flames_0.palette->data[2]; 
	palette_bg_3[0] = flames_0.palette->data[2]; 	

#ifdef FX_ENABLE_WRITER
	/* init writer */
	RSE_pgwriterRestart();
	pgwriter_clear_prev_tile = TRUE;
	pg_current_char_y = 1;
	RSE_pgwriterSetInitialY(pg_current_char_y);
	pgwriter_display_duration = 20;
	current_strings = (char **)strings_greets_0;
 #endif

	vcount = 0;
	vcount2 = 0;
	k = 0;
	logo_idx = 0;
	spr_max = LOGO_VLEN_0;
	logo_xy = logo_0;
	letter_state = 0;
	letter_transition = MAX_FADE_SPRITE;
	logo_on_top = FALSE;
	should_exit = FALSE;
	logos_done = FALSE;
	l = easing_table[(32 - letter_transition) * (EASING_TABLE_LEN - 1) >> 5];
	l = l >> 3;
	VDP_setVerticalScroll(PLAN_A, l - (1024 >> 3));

	while(!should_exit || VDP_isDoingFade())
	{
		VDP_waitVSync();
		DMA_flushQueue();
		music_sync = music_getElapsed();			

		if (vcount2 > 0 || RSE_pgwriterIsDone())
			vcount2++;

		if (vcount == RSE_FRAMES(16) + 2)
			VDP_fadePalTo(PAL0, shield_anim_0.palette->data, RSE_FRAMES(16), TRUE);
		else
		/* wipe fx */
		if (vcount > RSE_FRAMES(40) && vcount < RSE_FRAMES(80))
		{
			si = vcount - RSE_FRAMES(40);
			sj = (si * 700) / RSE_FRAMES(80 - 40);
			for (i = 0; i < 32; i++)
			{
				si = sj + i - 20;
				if (si < -700) si = -700;
				wipe_tile_x[i] = -si;
			}
			VDP_setHorizontalScrollTile(PLAN_A, 0, wipe_tile_x, 8 << 2, TRUE);
		}
		else
		/* clear the wipe fx layer */
		if (vcount >= RSE_FRAMES(80) && vcount < RSE_FRAMES(80) + 32)
			RSE_clearTileRowAWithPrio(vcount - RSE_FRAMES(80));
		else
		/* reset the scroll offset */
		if (vcount == RSE_FRAMES(80) + 40)
		{
			for (i = 0; i < 32; i++)
				wipe_tile_x[i] = 0;
			VDP_setHorizontalScrollTile(PLAN_A, 0, wipe_tile_x, 8 << 2, TRUE);
		}

		if (vcount == RSE_FRAMES(70))
				VDP_fadePalTo(PAL0, palette_bg_0_start, RSE_FRAMES(16), TRUE);

		if (vcount == RSE_FRAMES(200))
			VDP_fadePalTo(PAL2, sim1_font.palette->data, RSE_FRAMES(16), TRUE);
		else
		if (vcount > RSE_FRAMES(200))
		{
			if (vcount == 230)
				VDP_fadePalTo(PAL3, ball_square.palette->data, RSE_FRAMES(16), TRUE);

#ifdef FX_ENABLE_WRITER
			RSE_pgwriterUpdateMultiLine();
#endif

			switch(letter_state)
			{
				// fade in
				case 0:
					if (music_sync > 4950)
					{
						letter_transition--;

						l = easing_table[(32 - letter_transition) * (EASING_TABLE_LEN - 1) >> 5];
						l = l >> 3;
						if (logo_on_top)
							VDP_setVerticalScroll(PLAN_A, -l);
						else
							VDP_setVerticalScroll(PLAN_A, l - (1024 >> 3));

						if (letter_transition <= 0)
						{
							letter_transition = 0;
							letter_state = 1;						
						}
					}
					break;

				// logo display
				case 1:					
					switch(logo_idx)
					{
						case 0:
							if (music_sync > 5125) // 5180 + 20)
								letter_state = 2;
							break;

						case 1:
							if (music_sync > 5325)
								letter_state = 2;
							break;

						case 2:
							if (music_sync > 5500)
								letter_state = 2;
							break;

						case 3:
							if (music_sync > 5700)
								letter_state = 2;
							break;

						case 4:
							if (music_sync > 5900)
								letter_state = 2;
							break;

						case 5:
							if (music_sync > 6100)
								letter_state = 2;
							break;

						case 6:
							if (music_sync > 6300)
								letter_state = 2;
							break;

						case 7:
							if (music_sync > 6500)
								letter_state = 2;
							break;

						case 8:
							if (music_sync > 6700)
								letter_state = 2;
							break;

						case 9:
							if (music_sync > 7000)
								letter_state = 2;
							break;								
					}

				break;

				// logo fade out
				case 2:
					letter_transition++;
					if (letter_transition >= MAX_FADE_SPRITE)
					{
						letter_transition = MAX_FADE_SPRITE;
						letter_state = 3; 
					}
					break;

				//	Logo change
				case 3:
					logo_idx++;

					if (logo_idx >= MAX_VBALL_LOGO)
					{
						logo_idx = 0;
						logos_done = TRUE;
					}

					switch(logo_idx)
					{
						case 0:
							spr_max = LOGO_VLEN_0;
							logo_xy = logo_0;
							break;

						case 1:
							spr_max = LOGO_VLEN_1;
							logo_xy = logo_1;
							break;

						case 2:
							spr_max = LOGO_VLEN_2;
							logo_xy = logo_2;
							break;

						case 3:
							spr_max = LOGO_VLEN_3;
							logo_xy = logo_3;
							break;

						case 4:
							spr_max = LOGO_VLEN_4;
							logo_xy = logo_4;
							break;

						case 5:
							spr_max = LOGO_VLEN_5;
							logo_xy = logo_5;
							break;

						case 6:
							spr_max = LOGO_VLEN_6;
							logo_xy = logo_6;
							break;

						case 7:
							spr_max = LOGO_VLEN_7;
							logo_xy = logo_7;
							break;

						case 8:
							spr_max = LOGO_VLEN_8;
							logo_xy = logo_8;
							break;

						case 9:
							spr_max = LOGO_VLEN_9;
							logo_xy = logo_9;
							break;								
					}

#ifdef FX_ENABLE_SPRITES
					for(i = 0; i < MAX_SPRITE; i++)
					{	
						sprites[i]->x = 0;
						sprites[i]->y = 0;
						sprites[i]->status |= 0x0002;
						// SPR_setFrame(sprites[i], 12);
						// SPR_setPosition(sprites[i], -80, -80);
					}
#endif

					switch (logo_idx & 0x3)
					{
						case 0:
							VDP_setPalette(PAL3, square_pal_1.palette->data);
							// VDP_setPalette(PAL0, palette_bg_0);
							VDP_fadePalTo(PAL0, palette_bg_0, RSE_FRAMES(8), TRUE);
							break;
						case 1:
							VDP_setPalette(PAL3, square_pal_2.palette->data);
							// VDP_setPalette(PAL0, palette_bg_1);
							VDP_fadePalTo(PAL0, palette_bg_1, RSE_FRAMES(8), TRUE);
							break;
						case 2:
							VDP_setPalette(PAL3, square_pal_3.palette->data);
							// VDP_setPalette(PAL0, palette_bg_2);
							VDP_fadePalTo(PAL0, palette_bg_2, RSE_FRAMES(8), TRUE);
							break;
						case 3:
							VDP_setPalette(PAL3, square_pal_4.palette->data);
							// VDP_setPalette(PAL0, palette_bg_3);
							VDP_fadePalTo(PAL0, palette_bg_3, RSE_FRAMES(8), TRUE);
							break;
					}

					logo_on_top = !logo_on_top;
					letter_state = 0;
					break;	
			}

			// if (letter_state != 4)
#ifdef FX_ENABLE_SPRITES
			if (!logos_done)
			{
				for(i = 0; i < spr_max << 1; i+=2)
				{
					sprites[i >> 1]->x = logo_xy[i];

					if (logo_on_top)
						sprites[i >> 1]->y = logo_xy[i + 1] + ((cosFix16((logo_xy[i] + vcount) << 3)) >> 4) - 40;
					else
						sprites[i >> 1]->y = logo_xy[i + 1] + ((cosFix16((logo_xy[i] + vcount) << 3)) >> 4) + 40;

					// sprites[i >> 1]->status |= 0x0002;			

					// SPR_setFrame(sprites[i >> 1], max(0, min(11, ((cosFix16((logo_xy[i + 1] + logo_xy[i] + vcount) << 4) + 64) >> 5) + letter_transition)));
					SPR_setVRAMTileIndex(sprites[i >> 1], tileIndexes[min(11, ((cosFix16((logo_xy[i + 1] + logo_xy[i] + vcount) << 4) + 64) >> 5) + letter_transition)]);
					sprites[i >> 1]->status |= 0x0042;
				}
			}

			SPR_update(spr_max);
 #endif
		}

		/* Animate the shields in the background */
		if (k > 2)
		{			
			k = 0;
			for (i = 0; i < 4; i++)
				for(j = 0; j < 8; j++)
					shield_hscroll[(i << 3) + j] += scroll_dir[(i << 3) + j];
		}

		for(i = 0; i < 32; i++)
		{
			j = i >> 3;
			scroll_tile_x[i] = shield_hscroll[i] + sinFix16((vcount + (j << 4)) << 3);
		}

		VDP_setHorizontalScrollTile(PLAN_B, 0, scroll_tile_x, 8 << 2, TRUE);

		vcount++;
		k++;

		/* exit transition */
		if (logos_done && letter_state == 1 && RSE_pgwriterIsDone()) // (vcount2 > RSE_FRAMES(30))
		{
			// if (vcount2 == RSE_FRAMES(30) + 5)
			if (!should_exit)
			{
				VDP_fadeOut(0, 63, 16, TRUE);
				should_exit = TRUE;
			}

			for (i = 0; i < 32; i++)
				if (i & 0x1)
					wipe_tile_x[i] += (i >> 1) + 1;
				else
					wipe_tile_x[i] -= ((i >> 1) + 1);

			VDP_setHorizontalScrollTile(PLAN_A, 0, wipe_tile_x, 8 << 2, TRUE);			
		}

	}

	RSE_clearAll();
}