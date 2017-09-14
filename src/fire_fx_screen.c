#include "genesis.h"
#include <gfx.h>
#include "transition_helper.h"
#include "easing_table.h"
#include "firefx.h"
#include "fb.h"
#include "music.h"

extern u16 vramIndex;
// extern u16 fontIndex;
extern u8 framerate;

static u16 tileIndexes[64];

#define SCROLL_OFFSET 64
#define SCROLL_LENGTH 58
#define PENTACLE_HTILE 15

static u16 *pentacle_x, *pentacle_y;

NOINLINE void fireFX(void)
{
	u16 i, vcount, pentacle_y_offset;
	u16 scroll_easing;
	u16 firePal[64] =
	{
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		
		0x000, 0x500, 0xA00, 0xF00,
		0xF50, 0xF05, 0xA05, 0xA0A,
		0x00A, 0x50F, 0x00F, 0x05F,
		0x0AF, 0x5FF, 0xAFF, 0xFFF,
	};
	s16 fadeInTime = RSE_FRAMES(16);
	s16 scroll[FB_MAX_WIDTH / 16];
	u8 bgColorSave;
	u32 music_sync = 0;

	static Sprite *sprites[PENTACLE_HTILE];

	auto void inline updatePentacle(u16 y_offset)
	{
		u16 k;

		if (y_offset == 0)
		{
			for(k = 0; k < PENTACLE_HTILE; k++)
			{
				sprites[k]->y = pentacle_y[vcount + k];
				sprites[k]->status |= 0x0002;
			}
		}
		else
			for(k = 0; k < PENTACLE_HTILE; k++)
			{
				sprites[k]->y = pentacle_y[vcount + k] + y_offset;
				sprites[k]->status |= 0x0002;
			}

		SPR_update(PENTACLE_HTILE);
	};

	pentacle_x = MEM_alloc(1024 * sizeof(u16));
	pentacle_y = MEM_alloc(1024 * sizeof(u16));
	
	VDP_waitVSync(); // start on a clean VSync
	VDP_setEnable(0);

	RSE_changeResolution(256);

	VDP_setPlanSize(32, 64);
	VDP_setScrollingMode(HSCROLL_PLANE, VSCROLL_PLANE);

	VDP_drawImageEx(PLAN_A, &conan_on_fire, TILE_ATTR_FULL(PAL0, TRUE, FALSE, FALSE, vramIndex), 1, 0, FALSE, TRUE);
	vramIndex += conan_on_fire.tileset->numTile;
	DMA_flushQueue();
	
	memcpy(firePal, conan_on_fire.palette->data, 16 * sizeof(u16));
	memcpy(&firePal[16], fire_gradient.palette->data, 16 * sizeof(u16));

	/* pentacle */
	SPR_init(PENTACLE_HTILE, 0, 0);

    for(i = 0; i < PENTACLE_HTILE; i++)
    {
        TileSet* tileset = pentacle.animations[0]->frames[i]->tileset;

        VDP_loadTileSet(tileset, vramIndex, TRUE);
        tileIndexes[i] = vramIndex;
        vramIndex += tileset->numTile;
    }	
	DMA_flushQueue();

	s16 spos = VDP_allocateSprites(PENTACLE_HTILE * pentacle.maxNumSprite);
	for(i = 0; i < PENTACLE_HTILE; i++)
	{
		u16 ns = pentacle.animations[0]->frames[i]->numSprite;
		sprites[i] = SPR_addSpriteEx(&pentacle, (i << 3) + 64, 0, TILE_ATTR_FULL(PAL2, FALSE, FALSE, FALSE, vramIndex), spos , 0);
		SPR_setFrame(sprites[i], i);
		SPR_setAlwaysVisible(sprites[i], TRUE);
		SPR_setVRAMTileIndex(sprites[i], tileIndexes[i]);
		sprites[i]->lastVDPSprite -= pentacle.maxNumSprite - ns;
		spos += ns;
	}

	for(i = 0; i < 1024; i++)
	{
		pentacle_x[i] = 0x80 + 64 + (i << 3);
		pentacle_y[i] = 0x80 + 12 + (sinFix16((i) << 5) / 12);
	}

	SPR_update(PENTACLE_HTILE);

	RSE_turn_screen_to_black();
	bgColorSave = VDP_getBackgroundColor();
	VDP_setBackgroundColor(16);

	// initial barb scroll position
	memsetU16((u16 *)scroll, SCROLL_OFFSET, FB_MAX_WIDTH / 16);
	VDP_setVerticalScrollTile(PLAN_A, 0, scroll, FB_MAX_WIDTH / 16, FALSE);

	firefx_init(PLAN_B, PAL1);

	DMA_flushQueue();
	VDP_setEnable(1);

	VDP_fadeAll(palette_black, firePal, fadeInTime, TRUE);

	vcount = 0;
	while(music_sync < 4705)
	{
		music_sync = music_getElapsed();

		if (vcount < SCROLL_LENGTH)
		{
			scroll_easing = vcount + (64 - SCROLL_LENGTH);
			scroll_easing = easing_table[scroll_easing << 4] >> 5;
			scroll_easing = min(scroll_easing, SCROLL_LENGTH);
			scroll_easing = SCROLL_OFFSET - scroll_easing;

			memsetU16((u16 *)scroll, scroll_easing, FB_MAX_WIDTH / 16);
			VDP_setVerticalScrollTile(PLAN_A, 0, scroll, FB_MAX_WIDTH / 16, FALSE);
		}
		else
			updatePentacle(0);

		if (vcount == SCROLL_LENGTH)
			VDP_fadePal(PAL2, &firePal[32], pentacle.palette->data, RSE_FRAMES(32), TRUE);

		if (vcount < SCROLL_LENGTH && VDP_isDoingFade())
		{
			// firefx is 30fps...
			VDP_waitVSync();
			VDP_waitVSync();
		}
		else
			firefx_loop();
		
		DMA_flushQueue();

		vcount++;
	}

	/* fade out the fire & barbarian */
	VDP_fadeOut(0, 31, 16, TRUE);

	/* Wipe the barbarian */
	i = 0;
	while(i < RSE_FRAMES(64))
	{

		if (i == RSE_FRAMES(64 - 16 - 6))
			VDP_fadePalTo(PAL2, palette_white_bg, RSE_FRAMES(6), TRUE);
		else
		if (i == RSE_FRAMES(64 - 16))
			VDP_fadeOut(32, 32 + 15, RSE_FRAMES(16), TRUE);
		// firefx is 30fps...
		VDP_waitVSync();

		pentacle_y_offset = easing_table[i << 4] >> 5;
		// KLog_U1("pentacle_y_offset = ", pentacle_y_offset);
		updatePentacle(pentacle_y_offset);
		DMA_flushQueue();

		if (i < RSE_FRAMES(32))
		{
			RSE_clearTileRowA(i);
			RSE_clearTileRowA(32 - i);
		}

		i++;
		if (i & 0x1)
			vcount++;
	}

	MEM_free(pentacle_x);
	MEM_free(pentacle_y);
	firefx_close();

	VDP_setBackgroundColor(bgColorSave);

	RSE_clearAll();

// RSE_turn_screen_to_color(0xF0F);
	while(music_sync < 4735) // vcount < fxEnd)
	{
		VDP_waitVSync();
		music_sync = music_getElapsed();
	}
// RSE_turn_screen_to_color(0x0FF);
}