#include "genesis.h"
#include "transition_helper.h"
#include "res/gfx.h"
#include "easing_table_fp.h"
#include "music.h"

#define SCREEN_HEIGHT 224

#define SKY_START_Y 32
#define BIRD_X 48
#define TRUNK_TOP_X (140-4)
#define TRUNK_TOP_Y (-26-12)
#define TRUNK_BTM_X (120-4)
#define TRUNK_BTM_Y (102-12)
#define VILLAGE_Y (136+4)
#define SYSTILES_Y_OFF 32
#define PLAN_B_Y_SPLIT 104

#define FX_LEN RSE_FRAMES(60 * 24)
#define INIT_FADE RSE_FRAMES(32)
#define SCROLL_LEN RSE_FRAMES(60 * 4)
#define BIRD_FADE_START RSE_FRAMES(60 * 6)
#define BIRD_FADE_LEN RSE_FRAMES(32) // 2 times (to white, to bird)
#define END_FADE_WHITE RSE_FRAMES(32) // 4 times (sky, bird, village, white)
#define END_FADE_BLACK RSE_FRAMES(32)

extern u16 vramIndex;
extern u16 fontIndex;
extern u8 framerate;

static u16 ypos[5] = {0};

static void hInt(void)
{
	u8 lineidx = GET_VCOUNTER;
	
	if (lineidx >= ypos[2])
	{
		VDP_setVerticalScroll(PLAN_B, ypos[1]);
	}
	else if (lineidx >= ypos[3] && lineidx < ypos[2])
	{
		VDP_setVerticalScroll(PLAN_B, VILLAGE_Y - lineidx);
	}
}
 
static void vInt(void)
{
	VDP_setVerticalScroll(PLAN_B, ypos[4]);
}

static void makeYPos(s16 vcount)
{
	u16 tbl = easing_table[vcount * EASING_TABLE_LEN / SCROLL_LEN];	
	ypos[0] = tbl >> 4;
	ypos[1] = ypos[0] >> 1;
	ypos[2] = PLAN_B_Y_SPLIT + SYSTILES_Y_OFF - (ypos[1] >> 1);
	ypos[3] = (tbl * 7) >> 6;
	ypos[4] = (rm_layer_bird.map->h << 3) + SYSTILES_Y_OFF - ypos[3];
}

NOINLINE void owlFX(void)
{
	u16 pal[64];
	s16 hscroll[SCREEN_HEIGHT];
	u32 music_sync = 0;
	
	_voidCallback *hIntSave = internalHIntCB;
	u16 aAddrSave = VDP_getAPlanAddress();
	u16 bAddrSave = VDP_getBPlanAddress();
	u16 sAddrSave = VDP_getSpriteListAddress();
	u16 hAddrSave = VDP_getHScrollTableAddress();
	
	VDP_waitVSync(); // start on a clean VSync
	VDP_setEnable(0);
	
	RSE_changeResolution(320);

	VDP_setPlanSize(64, 32);
	VDP_setScrollingMode(HSCROLL_LINE, VSCROLL_PLANE);
	maps_addr = 0x0000; // prevent SGDK from loading font
	VDP_setBPlanAddress(0x0000);
	VDP_setAPlanAddress(0xE000);
	VDP_setHScrollTableAddress(0xF000);
	VDP_setSpriteListAddress(0xF400);
	vramIndex = 0x1000 / TILE_SIZE;
		
	VDP_clearPlan(PLAN_B, TRUE);
	VDP_clearPlan(PLAN_A, TRUE);
	VDP_clearPlan(PLAN_WINDOW, TRUE);

	VDP_drawImageEx(PLAN_A, &rm_layer_sky, TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, vramIndex), 0, 0,  FALSE, TRUE);
	vramIndex += rm_layer_sky.tileset->numTile;
	VDP_drawImageEx(PLAN_B, &rm_layer_village, TILE_ATTR_FULL(PAL1, TRUE, FALSE, FALSE, vramIndex), 0, VILLAGE_Y / 8,  FALSE, TRUE);
	vramIndex += rm_layer_village.tileset->numTile;
	DMA_flushQueue();
	VDP_drawImageEx(PLAN_B, &rm_layer_bird, TILE_ATTR_FULL(PAL2, TRUE, FALSE, FALSE, vramIndex), BIRD_X / 8, SYSTILES_Y_OFF / 8,  FALSE, TRUE);
	vramIndex += rm_layer_bird.tileset->numTile;
	DMA_flushQueue();
	
	SPR_init(0, 0, 0);
	Sprite * trunk_top = SPR_addSpriteEx(&rm_layer_trunk_top, TRUNK_TOP_X, TRUNK_TOP_Y,
			TILE_ATTR_FULL(PAL1, TRUE, FALSE, FALSE, 0xF600 / TILE_SIZE), 0, SPR_FLAG_AUTO_SPRITE_ALLOC | SPR_FLAG_AUTO_TILE_UPLOAD);
	SPR_setVisibility(trunk_top, VISIBLE);

	Sprite * trunk_btm = SPR_addSpriteEx(&rm_layer_trunk_btm, TRUNK_BTM_X, TRUNK_BTM_Y,
			TILE_ATTR_FULL(PAL1, FALSE, FALSE, FALSE, vramIndex), 0, SPR_FLAG_AUTO_SPRITE_ALLOC | SPR_FLAG_AUTO_TILE_UPLOAD);
	vramIndex += rm_layer_trunk_btm.maxNumTile;
	SPR_setVisibility(trunk_btm, VISIBLE);
	
	s16 tilesLeft = VDP_getAPlanAddress() / TILE_SIZE - vramIndex;
	if (tilesLeft < 0)
	{
		char s[30] = "VRAM overflow: ";
		intToStr(-tilesLeft, &s[15], 0);
		SYS_die(s);
	}
	
	SPR_update();
	VDP_setVerticalScroll(PLAN_B, (rm_layer_bird.map->h << 3));
	VDP_setVerticalScroll(PLAN_A, SKY_START_Y);
	
	memsetU16(pal, 0, 64);
	memcpyU16(&pal[0], rm_layer_sky.palette->data, 16);
	memcpyU16(&pal[16], rm_layer_village.palette->data, 16);
	
	for (u16 i = 0; i < 8; ++i)			
	{
		pal[63 - i] = pal[48 + i] = rm_layer_sky.palette->data[i + 3];
	}
	
	memsetU16((u16 *) hscroll, 0, SCREEN_HEIGHT);
	VDP_setHorizontalScrollLine(PLAN_A, 0, hscroll, SCREEN_HEIGHT, TRUE);
	VDP_setHorizontalScrollLine(PLAN_B, 0, hscroll, SCREEN_HEIGHT, TRUE);
	
	VDP_setHIntCounter(3);
	VDP_setHInterrupt(1);
	internalHIntCB = hInt;
	SYS_setVIntCallback(vInt);
	
	makeYPos(0);
	
	DMA_flushQueue();
	VDP_setEnable(1);

	u16 vcount = 0;
	u16 fxLen = FX_LEN;
	u16 initFade = INIT_FADE;
	u16 endFade = END_FADE_WHITE;
	u16 birdFadeStart = BIRD_FADE_START;
	u16 birdFade = BIRD_FADE_LEN;
	u16 scrollLen = SCROLL_LEN;
	u16 sineEnd = birdFadeStart + birdFade * 2;
	u8 fading;

	// music_sync = music_getElapsed();
	// KLog_U2("music_sync = ", music_sync, "vcount = ", vcount);	
	while(music_sync < 9050)
	{
		VDP_waitVSync();
		music_sync = music_getElapsed();
	}
	// music_sync = music_getElapsed();
	// KLog_U2("music_sync = ", music_sync, "vcount = ", vcount);		
		
	VDP_initFading(0, 31, palette_black, &pal[0], initFade, FALSE);
	fading = 1;
	
	while(vcount < fxLen)
	{
		VDP_waitVSync();
		DMA_flushQueue();
		
		if (fading)
			fading = VDP_doStepFading(0);

		if (vcount > initFade && vcount < birdFadeStart)
		{
			for (u16 i = 32; i < 48; ++i)			
				pal[i] = pal[48 + ((i + (vcount >> 1)) % 16)];
			
			VDP_setPalette(PAL2, &pal[32]);
		}
		else if (vcount == birdFadeStart)
		{
			VDP_initFading(32, 47, &pal[32], palette_white, birdFade, TRUE);
			fading = 1;
		}
		else if (vcount == birdFadeStart + birdFade)
		{
			VDP_interruptFade();
			VDP_initFading(32, 47, palette_white, rm_layer_bird.palette->data, birdFade, TRUE);
			fading = 1;
		}
		else if (vcount == fxLen - endFade * 4)
		{
			VDP_initFading(0, 15, rm_layer_sky.palette->data, palette_white, endFade >> 1, TRUE);
			fading = 1;
		}
		else if (vcount == fxLen - endFade * 3)
		{
			VDP_initFading(32, 47, rm_layer_bird.palette->data, palette_white, endFade >> 1, TRUE);
			fading = 1;
		}
		else if (vcount == fxLen - endFade * 2)
		{
			VDP_initFading(16, 31, rm_layer_village.palette->data, palette_white, endFade >> 1, TRUE);
			fading = 1;
		}

		if (vcount >= initFade && vcount < scrollLen + initFade)
		{
			makeYPos(vcount - initFade);
			
			trunk_top->y = 128 + TRUNK_TOP_Y + (ypos[1] >> 1);
			trunk_top->status |= 0x0002;
			trunk_btm->y = 128 + TRUNK_BTM_Y + (ypos[1] >> 1);
			trunk_btm->status |= 0x0002;
			SPR_update();

			VDP_setVerticalScroll(PLAN_A, SKY_START_Y - ypos[1]);
		}
		
		if (vcount <= sineEnd)
		{
			for (s16 i = 0; i < ypos[3]; i += 2)
			{
				u16 alpha  = (((ypos[3] - i) << 1) + vcount) << 4;
				fix16 mul = (cosFix16(vcount - 192) * (sineEnd - vcount)) >> 4;
				hscroll[i] =  fix16ToInt(fix16Mul(sinFix16(alpha), mul));
				hscroll[i + 1] = fix16ToInt(fix16Mul(cosFix16(alpha), mul));
			}
			VDP_setHorizontalScrollLine(PLAN_B, 0, hscroll, ypos[3], FALSE);
		}

		vcount += 2;
	}
	VDP_waitFadeCompletion();
	
	VDP_fadeTo(0, 47, palette_black, END_FADE_BLACK, FALSE);
	
	internalHIntCB = hIntSave;

	VDP_setAPlanAddress(aAddrSave);
	VDP_setHScrollTableAddress(hAddrSave);
	VDP_setBPlanAddress(bAddrSave);
	VDP_setSpriteListAddress(sAddrSave);
	
	RSE_reuploadSystemTiles();

	RSE_clearAll();
}
