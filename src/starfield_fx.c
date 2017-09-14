#include "genesis.h"
#include "transition_helper.h"
#include "div_premul.h"
#include "demo_strings.h"
#include "page_writer.h"
#include "scroll_h_table.h"
#include "scroll_v_table.h"
#include <gfx.h>

#define MAX_STAR  60
#define STARFIELD_SIZE (0x40)
#define STARFIELD_DIST -32
#define X_SCREEN_CENTER (((320 - 16) >> 1) + 0x80)
#define Y_SCREEN_CENTER (((224 - 32) >> 1) + 0x80)
#define SCR_TILE_W (320 >> 4)
#define SCR_TILE_H (100 >> 3)
#define SCR_SCROLL_STEP_W 16
#define SCR_SCROLL_STEP_H 8
#define MODE7_AMPLITUDE 1

typedef struct
{
	s16 x,y,z;
} _star;

extern u16 vramIndex;
// extern u16 fontIndex;
extern u8 framerate;

static u16 tileIndexes[16];
static _star stars[MAX_STAR];
static Sprite *sprites[MAX_STAR];

// static s16 scroll_v[SCR_TILE_W * SCR_SCROLL_STEP_W];
// static s16 scroll_h[SCR_TILE_H * SCR_SCROLL_STEP_H];

NOINLINE void RSE_Starfield_3D_Spr(void)
{
	// u16 i, j;
	u16 vcount;
	u16 loop;
	s16 numstars;

	/*	Initialize the list of stars */
	auto void initStar(s16 num)
	{
		_star *p;
		s16 i;

		numstars = num;

		i = num;
		p = stars;
		while(i--)
		{
			p->x = (random() % STARFIELD_SIZE) - (STARFIELD_SIZE >> 1);
			p->y = (random() % STARFIELD_SIZE) - (STARFIELD_SIZE >> 1);
			p->z = (random() % STARFIELD_SIZE) - (STARFIELD_SIZE >> 1) + STARFIELD_DIST;
			p++;
		}
	};

	/*	Draw the stars */
	auto void inline updateAndDrawStar(_star *part, s16 num)
	{
		_star *p;
		s16 i;
		s16 zp, x, y;

		i = num;
		p = part;

		while(i--)
		{
			p->z++;
			if (p->z > (STARFIELD_SIZE >> 1) + STARFIELD_DIST)
			{
				p->z -= ((STARFIELD_SIZE >> 1) - STARFIELD_DIST);
				p->x = (random() % STARFIELD_SIZE) - (STARFIELD_SIZE >> 1);
				p->y = (random() % STARFIELD_SIZE) - (STARFIELD_SIZE >> 1);
			}

			if (p->z != 0)
			{

				x = (p->x << 8);
				y = (p->y << 7);
				zp = p->z - 45;

				if (zp > 0 && zp < DIV_PREMUL_LEN)
					zp = div_table[zp];
				else
				if (zp < 0 && zp > -DIV_PREMUL_LEN)
					zp = -div_table[-zp];
				else
				if (zp < 0)
					zp = -((u16)65535/(u16)(-zp));
				else
					zp = (u16)65535/(u16)(zp);

				x = (x * zp) >> 16;
				y = (y * zp) >> 16;

				sprites[i]->x = X_SCREEN_CENTER + x;
				sprites[i]->y = Y_SCREEN_CENTER + y;
				sprites[i]->status |= 0x0002;
			}
			p++;
		}

		SPR_update(sprites, MAX_STAR);
		DMA_flushQueue();
	};

	VDP_setEnable(0);

	RSE_changeResolution(320);

	VDP_setPlanSize(64, 32);
	VDP_setWindowAddress(0x6000);
	VDP_clearPlan(PLAN_WINDOW, TRUE);
	
	vramIndex = TILE_USERINDEX;
/*	fontIndex = */RSE_pgwriterSetup();

	// vramIndex = fontIndex;

	for(loop = 0; loop < 4; loop++)
	{
		VDP_drawImageEx(PLAN_B, &bg_sky, TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, vramIndex), loop * 10, (224 - 160) >> 4, FALSE, TRUE);
	}

	vramIndex += bg_sky.tileset->numTile;

	VDP_drawImageEx(PLAN_A, &rse_retrowave_logo, TILE_ATTR_FULL(PAL1, TRUE, FALSE, FALSE, vramIndex), (320 - 140) >> 4, 4, FALSE, TRUE);
	vramIndex += rse_retrowave_logo.tileset->numTile;

	DMA_flushQueue();
	VDP_setEnable(1);

	SPR_init(MAX_STAR, 0, 0);

    for(loop = 0; loop < sprite_stars.animations[0]->numFrame; loop++)
    {
        TileSet* tileset = sprite_stars.animations[0]->frames[loop]->tileset;

        VDP_loadTileSet(tileset, vramIndex, TRUE);
        tileIndexes[loop] = vramIndex;
        vramIndex += tileset->numTile;
    }		

	/*	Initialize the needed amount of sprites */
	for(loop = 0; loop < MAX_STAR; loop++)
	{
		sprites[loop] = SPR_addSpriteEx(&sprite_stars, 0, 0, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, tileIndexes[loop & 0x7]), 0, !SPR_FLAG_AUTO_VISIBILITY | !SPR_FLAG_AUTO_VRAM_ALLOC | SPR_FLAG_AUTO_SPRITE_ALLOC | !SPR_FLAG_AUTO_TILE_UPLOAD);
		if (!sprites[loop])
		{
			KLog_U1("NULL sprite at ", loop);
			SYS_die("NULL sprite!");
		}
	}

	SPR_update(sprites, MAX_STAR);
	DMA_flushQueue();

	/* Initialise stars */
	initStar(MAX_STAR);

	/* init writer */
	RSE_pgwriterRestart();
	pg_initial_char_y = 11;
	pg_current_char_y = pg_initial_char_y;
	pgwriter_display_duration = 20;
	RSE_pgwriterSetOption(WRT_OPT_HALF_SPEED);
	pg_current_plan = PLAN_WINDOW;
	current_strings = (char **)strings_credits;
	VDP_setWindowHPos(FALSE, 0);
	VDP_setWindowVPos(TRUE, 12);

	VDP_fadePalTo(PAL3, sprite_stars.palette->data, RSE_FRAMES(32), TRUE);
	VDP_setScrollingMode(HSCROLL_TILE, VSCROLL_2TILE);

	/* Main loop */
	vcount = 0;

	while(TRUE)
	{
		VDP_waitVSync();
		if (vcount == RSE_FRAMES(40))
			VDP_fadePalTo(PAL0, bg_sky.palette->data, RSE_FRAMES(16), TRUE);
		else
		if (vcount == RSE_FRAMES(60))
			VDP_fadePalTo(PAL1, palette_white_bg, RSE_FRAMES(8), TRUE);
		if (vcount == RSE_FRAMES(70))
			VDP_fadePalTo(PAL1, rse_retrowave_logo.palette->data, RSE_FRAMES(16), TRUE);		
		else
		if (vcount >= RSE_FRAMES(100))
		{
			// if (vcount == RSE_FRAMES(100))
			// 	VDP_fadePalTo(PAL2, sim1_font.palette->data, RSE_FRAMES(64), TRUE);

			RSE_pgwriterUpdateMultiLine();
		}

		// calculates stars position
		// draw stars
		updateAndDrawStar(stars, numstars);

		VDP_setVerticalScrollTile(PLAN_A, 0, (s16 *)&(scroll_v[(((cosFix16(vcount << 3) + 64) * (SCR_SCROLL_STEP_W - 1)) >> 7) * SCR_TILE_W]), SCR_TILE_W, FALSE);
		VDP_setHorizontalScrollTile(PLAN_A, 0, (s16 *)&(scroll_h[(((cosFix16(vcount << 3) + 64) * (SCR_SCROLL_STEP_H - 1)) >> 7) * SCR_TILE_H]), SCR_TILE_H, FALSE);

		vcount++;
	}
	
	RSE_clearAll();
}
