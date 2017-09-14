#include "transition_helper.h"
#include "db.h"

/* Full white palette */
const u16 palette_white[] = {	0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,
								0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,
								0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,
								0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE	}; 

const u16 palette_white_bg[] = {	0x000,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,
								0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,
								0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,
								0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE	}; 

const s16 tile_sc_table[256] = { 	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };


#define RSE_RESOL_CHANGE_TICKS 10

#define frval(fr, x) (((x) * (fr)) / 60)

#define frval32(fr, x)\
	frval(fr, (x) * 32 +  0), frval(fr, (x) * 32 +  1), frval(fr, (x) * 32 +  2), frval(fr, (x) * 32 +  3),\
	frval(fr, (x) * 32 +  4), frval(fr, (x) * 32 +  5), frval(fr, (x) * 32 +  6), frval(fr, (x) * 32 +  7),\
	frval(fr, (x) * 32 +  8), frval(fr, (x) * 32 +  9), frval(fr, (x) * 32 + 10), frval(fr, (x) * 32 + 11),\
	frval(fr, (x) * 32 + 12), frval(fr, (x) * 32 + 13), frval(fr, (x) * 32 + 14), frval(fr, (x) * 32 + 15),\
	frval(fr, (x) * 32 + 16), frval(fr, (x) * 32 + 17), frval(fr, (x) * 32 + 18), frval(fr, (x) * 32 + 19),\
	frval(fr, (x) * 32 + 20), frval(fr, (x) * 32 + 21), frval(fr, (x) * 32 + 22), frval(fr, (x) * 32 + 23),\
	frval(fr, (x) * 32 + 24), frval(fr, (x) * 32 + 25), frval(fr, (x) * 32 + 26), frval(fr, (x) * 32 + 27),\
	frval(fr, (x) * 32 + 28), frval(fr, (x) * 32 + 29), frval(fr, (x) * 32 + 30), frval(fr, (x) * 32 + 31),

#define frval64(fr, x) frval32(fr, (x) * 2 + 0) frval32(fr, (x) * 2 + 1)

const u16 framerate_60[2048] = {
	frval64(60,  0) frval64(60,  1) frval64(60,  2) frval64(60,  3)
	frval64(60,  4) frval64(60,  5) frval64(60,  6) frval64(60,  7)
	frval64(60,  8) frval64(60,  9) frval64(60, 10) frval64(60, 11)
	frval64(60, 12) frval64(60, 13) frval64(60, 14) frval64(60, 15)
	frval64(60, 16) frval64(60, 17) frval64(60, 18) frval64(60, 19)
	frval64(60, 20) frval64(60, 21) frval64(60, 22) frval64(60, 23)
	frval64(60, 24) frval64(60, 25) frval64(60, 26) frval64(60, 27)
	frval64(60, 28) frval64(60, 29) frval64(60, 30) frval64(60, 31)
};

const u16 framerate_50[2048] = {
	frval64(50,  0) frval64(50,  1) frval64(50,  2) frval64(50,  3)
	frval64(50,  4) frval64(50,  5) frval64(50,  6) frval64(50,  7)
	frval64(50,  8) frval64(50,  9) frval64(50, 10) frval64(50, 11)
	frval64(50, 12) frval64(50, 13) frval64(50, 14) frval64(50, 15)
	frval64(50, 16) frval64(50, 17) frval64(50, 18) frval64(50, 19)
	frval64(50, 20) frval64(50, 21) frval64(50, 22) frval64(50, 23)
	frval64(50, 24) frval64(50, 25) frval64(50, 26) frval64(50, 27)
	frval64(50, 28) frval64(50, 29) frval64(50, 30) frval64(50, 31)
};

u16 *framerate_adapter;

extern u16 vramIndex;

void RSE_recalculate_framerate_equivalent(u8 fr)
{
	if (fr == 50)
		framerate_adapter = (u16 *)framerate_50;
	else if (fr == 60)
		framerate_adapter = (u16 *)framerate_60;
	else
		SYS_die("Unknown framerate");
}

u16 RSE_colMul(u16 A, u16 B)
{
	u32 rA, rB, gA, gB, bA, bB;
	u16 col;

	rA = (A & COL_REDMASK) >> COL_REDSFT;
	rB = (B & COL_REDMASK) >> COL_REDSFT;

	gA = (A & COL_GREENMASK) >> COL_GREENSFT;
	gB = (B & COL_GREENMASK) >> COL_GREENSFT;

	bA = (A & COL_BLUEMASK) >> COL_BLUESFT;
	bB = (B & COL_BLUEMASK) >> COL_BLUESFT;

	rA = (rA * rB) / 0xE;
	gA = (gA * gB) / 0xE;
	bA = (bA * bB) / 0xE;

	col = ((rA << COL_REDSFT) & COL_REDMASK) | ((gA << COL_GREENSFT) & COL_GREENMASK) | ((bA << COL_BLUESFT) & COL_BLUEMASK);

	return col;
}

u16 RSE_colAdd(u16 A, u16 B)
{
	u32 rA, rB, gA, gB, bA, bB;
	u16 col;

	rA = (A & COL_REDMASK) >> COL_REDSFT;
	rB = (B & COL_REDMASK) >> COL_REDSFT;

	gA = (A & COL_GREENMASK) >> COL_GREENSFT;
	gB = (B & COL_GREENMASK) >> COL_GREENSFT;

	bA = (A & COL_BLUEMASK) >> COL_BLUESFT;
	bB = (B & COL_BLUEMASK) >> COL_BLUESFT;

	rA = min(rA + rB, 0xE);
	gA = min(gA + gB, 0xE);
	bA = min(bA + bB, 0xE);

	col = ((rA << COL_REDSFT) & COL_REDMASK) | ((gA << COL_GREENSFT) & COL_GREENMASK) | ((bA << COL_BLUESFT) & COL_BLUEMASK);

	return col;
}

u16 RSE_colSub(u16 A, u16 B)
{
	s32 rA, rB, gA, gB, bA, bB;
	u16 col;

	rA = (A & COL_REDMASK) >> COL_REDSFT;
	rB = (B & COL_REDMASK) >> COL_REDSFT;

	gA = (A & COL_GREENMASK) >> COL_GREENSFT;
	gB = (B & COL_GREENMASK) >> COL_GREENSFT;

	bA = (A & COL_BLUEMASK) >> COL_BLUESFT;
	bB = (B & COL_BLUEMASK) >> COL_BLUESFT;

	rA = max(rA - rB, 0x0);
	gA = max(gA - gB, 0x0);
	bA = max(bA - bB, 0x0);

	col = ((rA << COL_REDSFT) & COL_REDMASK) | ((gA << COL_GREENSFT) & COL_GREENMASK) | ((bA << COL_BLUESFT) & COL_BLUEMASK);

	return col;
}

u16 RSE_colDivInt(u16 A, u16 B)
{
	u32 rA, gA, bA;
	u16 col;

	rA = (A & COL_REDMASK) >> COL_REDSFT;
	gA = (A & COL_GREENMASK) >> COL_GREENSFT;
	bA = (A & COL_BLUEMASK) >> COL_BLUESFT;

	rA /= B;
	gA /= B;
	bA /= B;

	col = ((rA << COL_REDSFT) & COL_REDMASK) | ((gA << COL_GREENSFT) & COL_GREENMASK) | ((bA << COL_BLUESFT) & COL_BLUEMASK);

	return col;
}

void RSE_turn_screen_to_white(void)
{
	/* Turn whole palette to white */
	u16 i;
	for(i = 0; i < 63; i++)
	{
		VDP_setPaletteColor(i, 0xFFF);
	}

}

void RSE_turn_screen_to_black(void)
{
	/* Turn whole palette to black */
	u16 i;
	for(i = 0; i < 63; i++)
	{
		VDP_setPaletteColor(i, 0x000);
	}

}

void RSE_turn_screen_to_color(u16 col)
{
	/* Turn whole palette to the specified color */
	u16 i;
	
	for(i = 0; i < 63; i++)
		VDP_setPaletteColor(i, col);

}

void RSE_pause(u16 frames)
{
	while(--frames > 0)
		VDP_waitVSync();
}

void RSE_clearTileRowB(u16 row)
{
	u16 col, max_col;
	max_col = VDP_getPlanWidth(); 
	for(col = 0; col < max_col; col++)
		VDP_setTileMapXY(PLAN_B, TILE_ATTR_FULL(PAL0, 0, 0, 0, TILE_SYSTEMINDEX), col, row);
}

void RSE_clearTileRowBWithPrio(u16 row)
{
	u16 col, max_col;
	max_col = VDP_getPlanWidth(); 
	for(col = 0; col < max_col; col++)
		VDP_setTileMapXY(PLAN_B, TILE_ATTR_FULL(PAL0, TRUE, 0, 0, TILE_SYSTEMINDEX), col, row);
}

void RSE_clearTileRowA(u16 row)
{
	u16 col, max_col;
	max_col = VDP_getPlanWidth(); 
	for(col = 0; col < max_col; col++)
		VDP_setTileMapXY(PLAN_A, TILE_ATTR_FULL(PAL0, 0, 0, 0, TILE_SYSTEMINDEX), col, row);
}

void RSE_clearTileRowAWithPrio(u16 row)
{
	u16 col, max_col;
	max_col = VDP_getPlanWidth(); 
	for(col = 0; col < max_col; col++)
		VDP_setTileMapXY(PLAN_A, TILE_ATTR_FULL(PAL0, TRUE, 0, 0, TILE_SYSTEMINDEX), col, row);
}

void RSE_resetScrolling(void)
{
	VDP_setVerticalScrollTile(PLAN_A, 0, (s16 *) tile_sc_table, 20, FALSE);	
	VDP_setVerticalScrollTile(PLAN_B, 0, (s16 *) tile_sc_table, 20, FALSE);
	VDP_setHorizontalScrollLine(PLAN_A, 0, (s16 *) tile_sc_table, 256, TRUE);
	VDP_setHorizontalScrollLine(PLAN_B, 0, (s16 *) tile_sc_table, 256, TRUE);

	VDP_setScrollingMode(HSCROLL_PLANE, VSCROLL_PLANE);
	VDP_setHorizontalScroll(PLAN_B, 0);
	VDP_setHorizontalScroll(PLAN_A, 0);		
}

void RSE_clearAll(void)
{
	VDP_setEnable(0);

	RSE_turn_screen_to_black();
			
	SPR_end();

	RSE_resetScrolling();

	VDP_setHInterrupt(0);
	SYS_setHIntCallback(NULL);
	SYS_setVIntCallback(NULL);

	VDP_setHilightShadow(0);
	VDP_setWindowHPos(FALSE, 0);
	VDP_setWindowVPos(FALSE, 0);
	VDP_setPlanSize(64, 64);
	VDP_clearPlan(PLAN_A, 1);
	VDP_clearPlan(PLAN_B, 1);	
	VDP_clearPlan(PLAN_WINDOW, 1);	
	
	VDP_setEnable(1);
	
	vramIndex = TILE_USERINDEX;
	
//	KLog_U1("MEM_getFree ", MEM_getFree());
	checkDbs(1, 1);
}

void RSE_reuploadSystemTiles(void)
{
    u16 i = 16;
    while(i--) VDP_fillTileData(i | (i << 4), TILE_SYSTEMINDEX + i, 1, TRUE);
}

void RSE_changeResolution(u16 width)
{
	if (width != screenWidth)
	{
		u8 en = VDP_getEnable();
		
		if (en)
		{
			VDP_setEnable(0);
		}
		
		waitTick(RSE_RESOL_CHANGE_TICKS); // let VDP stabilize
		
		if (width == 256)
		{
			VDP_setScreenWidth256();
		}
		else if (width == 320)
		{
			VDP_setScreenWidth320();
		}

		waitTick(RSE_RESOL_CHANGE_TICKS); // let VDP stabilize

		if (en)
		{
			VDP_setEnable(1);
		}
	}
}


void RSE_Safe_SPR_setFrame(Sprite *sprite, s16 frame)
{
	if (frame >= sprite->animation->numFrame)
	{
		SPR_setVisibility(sprite, HIDDEN);
	}
	else
	{
		SPR_setVisibility(sprite, VISIBLE);
		SPR_setFrame(sprite, frame);
	}
}