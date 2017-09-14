#include "firefx.h"

#include "fb.h"

#define SCREEN_HEIGHT 224
#define FIRE_HEIGHT 40
#define FIRE_COMPUTED_WIDTH 96
#define FIRE_DIST_HEIGHT 72
#define FIRE_DIST_STEP 3

extern void FB_vInt_fullFB(void);


static void drawRandomDots(void)
{
	u16 i;

	for (i = 0; i < 16; ++i) {
		u32 * ptr;
		u16 x = random() % (FB_MAX_WIDTH / 8);
		ptr = (u32 *) &fb[(x + 1) * FIRE_HEIGHT + FIRE_HEIGHT - 2];
		*ptr = 0xffffffff;
	}
}

static void fireBlur(void)
{
	u16 i;
	
#if 1
	u8 *pori1 = &((u8 *)fb)[2 * FIRE_HEIGHT];
	u8 *pori2 = &((u8 *)fb)[2 * FIRE_HEIGHT + 1];
	u8 *pori3 = &((u8 *)fb)[4 * FIRE_HEIGHT];
	u8 *pori4 = &((u8 *)fb)[4 * FIRE_HEIGHT + 1];
	u32 zf = 0x0f0f0f0f;
	u32 fz = 0xf0f0f0f0;

#define DO_32_PX(offset)\
	asm volatile (\
		/* first column */\
		\
		/* load pixel data*/\
		"movep.l %c[xm1m](%[pori1]), %%d1\n"\
		"movep.l %c[yp1](%[pori1]), %%d2\n"\
		"move.l %%d2, %%d4\n"\
		"movep.l %c[yp2](%[pori1]), %%d3\n"\
		\
		/* precalc common value to both pixels */\
		"move.l %%d2, %%d0\n"\
		"and.l %[zf], %%d0\n"\
		"lsr.l #4, %%d2\n"\
		"and.l %[zf], %%d2\n"\
		"add.l %%d0, %%d2\n"\
		\
		/* high pixel */\
		"and.l %[zf], %%d1\n"\
		"add.l %%d2, %%d1\n"\
		"move.l %%d3, %%d0\n"\
		"lsr.l #4, %%d0\n"\
		"and.l %[zf], %%d0\n"\
		"add.l %%d0, %%d1\n"\
		\
		/* load pixel data */\
		"movep.l %c[xp1p](%[pori1]), %%d0\n"\
		"move.l %%d0, %%d5\n"\
		\
		/* low pixel */\
		"lsr.l #4, %%d0\n"\
		"and.l %[zf], %%d0\n"\
		"add.l %%d2, %%d0\n"\
		"and.l %[zf], %%d3\n"\
		"add.l %%d3, %%d0\n"\
		\
		/* merge pixels */\
		"lsl.l #2, %%d1\n"\
		"and.l %[fz], %%d1\n"\
		"lsr.l #2, %%d0\n"\
		"and.l %[zf], %%d0\n"\
		"or.l %%d0, %%d1\n"\
		\
		/* save pixel pair */\
		"movep.l %%d1, %c[ori](%[pori1])\n"\
		/* second column */	\
		\
		/* load pixel data*/\
		"move.l %%d5, %%d2\n"\
		"movep.l %c[yp2](%[pori2]), %%d3\n"\
		\
		/* precalc common value to both pixels */\
		"move.l %%d5, %%d0\n"\
		"and.l %[zf], %%d0\n"\
		"lsr.l #4, %%d5\n"\
		"and.l %[zf], %%d5\n"\
		"add.l %%d0, %%d5\n"\
		\
		/* high pixel */\
		"and.l %[zf], %%d4\n"\
		"add.l %%d5, %%d4\n"\
		"move.l %%d3, %%d0\n"\
		"lsr.l #4, %%d0\n"\
		"and.l %[zf], %%d0\n"\
		"add.l %%d0, %%d4\n"\
		\
		/* load pixel data */\
		"movep.l %c[xp1m](%[pori2]), %%d0\n"\
		"move.l %%d0, %%d1\n"\
		/* low pixel */\
		"lsr.l #4, %%d0\n"\
		"and.l %[zf], %%d0\n"\
		"add.l %%d5, %%d0\n"\
		"and.l %[zf], %%d3\n"\
		"add.l %%d3, %%d0\n"\
		\
		/* merge pixels */\
		"lsl.l #2, %%d4\n"\
		"and.l %[fz], %%d4\n"\
		"lsr.l #2, %%d0\n"\
		"and.l %[zf], %%d0\n"\
		"or.l %%d0, %%d4\n"\
		\
		/* save pixel pair */\
		"movep.l %%d4, %c[ori](%[pori2])\n"\
		/* third column */\
		\
		/* load pixel data*/\
		"move.l %%d2, %%d4\n"\
		"movep.l %c[yp2](%[pori3]), %%d3\n"\
		\
		/* precalc common value to both pixels */\
		"move.l %%d2, %%d0\n"\
		"and.l %[zf], %%d0\n"\
		"lsr.l #4, %%d2\n"\
		"and.l %[zf], %%d2\n"\
		"add.l %%d0, %%d2\n"\
		\
		/* high pixel */\
		"and.l %[zf], %%d1\n"\
		"add.l %%d2, %%d1\n"\
		"move.l %%d3, %%d0\n"\
		"lsr.l #4, %%d0\n"\
		"and.l %[zf], %%d0\n"\
		"add.l %%d0, %%d1\n"\
		\
		/* load pixel data */\
		"movep.l %c[xp1p](%[pori3]), %%d0\n"\
		"move.l %%d0, %%d5\n"\
		\
		/* low pixel */\
		"lsr.l #4, %%d0\n"\
		"and.l %[zf], %%d0\n"\
		"add.l %%d2, %%d0\n"\
		"and.l %[zf], %%d3\n"\
		"add.l %%d3, %%d0\n"\
		\
		/* merge pixels */\
		"lsl.l #2, %%d1\n"\
		"and.l %[fz], %%d1\n"\
		"lsr.l #2, %%d0\n"\
		"and.l %[zf], %%d0\n"\
		"or.l %%d0, %%d1\n"\
		\
		/* save pixel pair */\
		"movep.l %%d1, %c[ori](%[pori3])\n"\
		/* fourth column */	\
		\
		/* load pixel data*/\
		"movep.l %c[yp2](%[pori4]), %%d3\n"\
		\
		/* precalc common value to both pixels */\
		"move.l %%d5, %%d0\n"\
		"and.l %[zf], %%d0\n"\
		"lsr.l #4, %%d5\n"\
		"and.l %[zf], %%d5\n"\
		"add.l %%d0, %%d5\n"\
		\
		/* high pixel */\
		"and.l %[zf], %%d4\n"\
		"add.l %%d5, %%d4\n"\
		"move.l %%d3, %%d0\n"\
		"lsr.l #4, %%d0\n"\
		"and.l %[zf], %%d0\n"\
		"add.l %%d0, %%d4\n"\
		\
		/* load pixel data */\
		"movep.l %c[xp1m](%[pori4]), %%d0\n"\
		/* low pixel */\
		"lsr.l #4, %%d0\n"\
		"and.l %[zf], %%d0\n"\
		"add.l %%d5, %%d0\n"\
		"and.l %[zf], %%d3\n"\
		"add.l %%d3, %%d0\n"\
		\
		/* merge pixels */\
		"lsl.l #2, %%d4\n"\
		"and.l %[fz], %%d4\n"\
		"lsr.l #2, %%d0\n"\
		"and.l %[zf], %%d0\n"\
		"or.l %%d0, %%d4\n"\
		\
		/* save pixel pair */\
		"movep.l %%d4, %c[ori](%[pori4])\n"\
		: \
		: [zf] "d" (zf), [fz] "d" (fz),\
			[pori1] "a" (pori1), [pori2] "a" (pori2), [pori3] "a" (pori3), [pori4] "a" (pori4),\
			[ori] "i" ((offset) * 2),\
			[xm1m] "i" ((1 - FIRE_HEIGHT + (offset)) * 2 - 1),\
			[xp1m] "i" ((1 + FIRE_HEIGHT + (offset)) * 2 - 1),\
			[xp1p] "i" ((1 + (offset)) * 2 + 1),\
			[yp1] "i" ((1 + (offset)) * 2),\
			[yp2] "i" ((2 + (offset)) * 2)\
		: "d0", "d1", "d2", "d3", "d4", "d5"\
	);
	
	#define DO_8_COL(idx) \
		DO_32_PX((idx) * FIRE_HEIGHT + 0x00)\
		DO_32_PX((idx) * FIRE_HEIGHT + 0x04)\
		DO_32_PX((idx) * FIRE_HEIGHT + 0x08)\
		DO_32_PX((idx) * FIRE_HEIGHT + 0x0c)\
		DO_32_PX((idx) * FIRE_HEIGHT + 0x10)\
		DO_32_PX((idx) * FIRE_HEIGHT + 0x14)\
		DO_32_PX((idx) * FIRE_HEIGHT + 0x18)\
		DO_32_PX((idx) * FIRE_HEIGHT + 0x1c)\
		DO_32_PX((idx) * FIRE_HEIGHT + 0x20)\
		DO_32_PX((idx) * FIRE_HEIGHT + 0x24)

	for(i = 0; i < FIRE_COMPUTED_WIDTH; i += 32)
	{
		DO_8_COL(0)
		DO_8_COL(2)
		DO_8_COL(4)
		DO_8_COL(6)
		pori1 += FIRE_HEIGHT * 16;
		pori2 += FIRE_HEIGHT * 16;
		pori3 += FIRE_HEIGHT * 16;
		pori4 += FIRE_HEIGHT * 16;
	}
	
#else		
	u16 *pori = &fb[FIRE_HEIGHT];
	u16 *pxm1, *pxp1, *pyp2;
	u16 xm1, xp1, yp2;
	u16 xm11, xp14, yp11, yp12, yp13, yp14, yp21, yp22, yp23, yp24;
	u16 acc;
	
	pxm1 = &fb[1];
	pxp1 = &fb[2 * FIRE_HEIGHT + 1];
	pyp2 = &fb[FIRE_HEIGHT + 2];
	yp11 = yp12 = yp13 = yp14 = 0;

	for(i = 0; i < FIRE_HEIGHT * FB_MAX_WIDTH / 8; ++i) {
		xm1 = *pxm1++;
		xp1 = *pxp1++;
		yp2 = *pyp2++;

		xm11 = xm1 & 0xf;
		xp14 = (xp1 >> 12) & 0xf;
		
		yp11 = yp21;
		yp12 = yp22;
		yp13 = yp23;
		yp14 = yp24;
		
		yp21 = yp2 & 0xf;
		yp2 >>= 4;
		yp22 = yp2 & 0xf;
		yp2 >>= 4;
		yp23 = yp2 & 0xf;
		yp2 >>= 4;
		yp24 = yp2 & 0xf;
		
		acc =  (xm11 + yp14 + yp13 + yp24) & 0x3c;
		acc <<= 4;
		acc |= (yp14 + yp13 + yp12 + yp23) & 0x3c;
		acc <<= 4;
		acc |= (yp13 + yp12 + yp11 + yp22) & 0x3c;
		acc <<= 2;
		acc |= ((yp12 + yp11 + xp14 + yp21) & 0x3c) >> 2;
		
		*pori++ = acc;
	}
#endif			
}

static s16 waveScale[FIRE_DIST_HEIGHT / FIRE_DIST_STEP][FIRE_COMPUTED_WIDTH / 8];
static u32 *waveScalePtr;
static s16 waveScaleAlpha = 0;

static void computeWaveScale(s16 alpha) {
	for (s16 i = 0; i < FIRE_COMPUTED_WIDTH / 8; ++i) {
		fix16 ratio = fix16Add(fix16Mul(sinFix16((alpha << 5) + (i << 7)), FIX16(0.33)), FIX16(0.66));
		fix16 acc = fix16Add(FIX16(-SCREEN_HEIGHT + FIRE_HEIGHT), fix16Mul(ratio, FIX16(FIRE_DIST_HEIGHT / FIRE_DIST_STEP)));
		for (s16 j = 0; j < FIRE_DIST_HEIGHT / FIRE_DIST_STEP; ++j) {
			waveScale[j][i] = fix16ToInt(acc);
			acc = fix16Sub(acc, ratio);
		}
	}
}

static void hInt(void) {
#if 1
	// hardcoded for PLAN_B and FIRE_COMPUTED_WIDTH = 96px

	asm volatile (
		"move.w #0x8f04, (%[plc])\n"
		"move.l %[ad1], (%[plc])\n"
		"move.l (%[wsp])+, (%[pld])\n"
		"move.l (%[wsp])+, (%[pld])\n"
		"move.l (%[wsp])+, (%[pld])\n"
		"move.l %[ad2], (%[plc])\n"
		"move.l (%[wsp])+, (%[pld])\n"
		"move.l (%[wsp])+, (%[pld])\n"
		"move.l (%[wsp])+, (%[pld])\n"
		"move.w %[air], (%[plc])\n"
		: [wsp] "+a" (waveScalePtr)
		: [pld] "a" (GFX_DATA_PORT), [plc] "a" (GFX_CTRL_PORT),
			[air] "d" (VDP_getReg(0xf) | 0x8f00),
			[ad1] "i" (GFX_WRITE_VSRAM_ADDR(2)),
			[ad2] "i" (GFX_WRITE_VSRAM_ADDR((FB_MAX_WIDTH - FIRE_COMPUTED_WIDTH) / 4 + 2))
	);
		
#else
	VDP_setVerticalScrollTile(fb_plan, 0, (s16 *) waveScalePtr, FIRE_COMPUTED_WIDTH / 16, FALSE);
	waveScalePtr += FIRE_COMPUTED_WIDTH / 32;
	VDP_setVerticalScrollTile(fb_plan, (FB_MAX_WIDTH - FIRE_COMPUTED_WIDTH) / 16, (s16 *) waveScalePtr, FIRE_COMPUTED_WIDTH / 16, FALSE);
	waveScalePtr += FIRE_COMPUTED_WIDTH / 32;
#endif
}

static void hIntPrep(void) {
	VDP_setHIntCounter(FIRE_DIST_STEP - 1);
	internalHIntCB = hInt;
}

static void vInt(void) {
	internalHIntCB = hIntPrep;
	VDP_setHIntCounter((SCREEN_HEIGHT - FIRE_DIST_HEIGHT) / 2);
	waveScalePtr = (u32 *) waveScale;
	FB_vInt_fullFB();
}

static void firefx_preparePlan(u8 palette) {
	u16 i, j, pw, sz;
	u16* buf;
	
	pw = VDP_getPlanWidth();
	sz = pw * (FB_MAX_HEIGHT / 8);
	
	buf = (u16*)MEM_alloc(sz << 1);
	memsetU16(buf, 0, sz);
	
	for (j = 0; j < (fb_height >> 3); ++j) {
		for (i = 0; i < (FB_MAX_WIDTH / 8); ++i) {
			if (i >= (FB_MAX_WIDTH / 16)) {
				buf[pw * j + i] = TILE_ATTR_FULL(palette, 0, 0, 1, FB_TILES_OFFSET + (FB_MAX_HEIGHT / 8) * ((FB_MAX_WIDTH / 8) - 1 - i) + j);
			} else {
				buf[pw * j + i] = TILE_ATTR_FULL(palette, 0, 0, 0, FB_TILES_OFFSET + (FB_MAX_HEIGHT / 8) * i + j);
			}
		}
	}
	
	VDP_setTileMapData(FB_getPlanAddress(), buf, 0, sz, TRUE);
	
	MEM_free(buf);
}

void firefx_init(VDPPlan plan, u8 palette) {
	FB_init(plan, palette, FB_MAX_WIDTH / 2, FIRE_HEIGHT, FB_INIT_FULL_FRAME);
	firefx_preparePlan(palette);
	computeWaveScale(0);
	internalHIntCB = hIntPrep;
	SYS_setVIntCallback(vInt);
	VDP_setHIntCounter((SCREEN_HEIGHT - FIRE_DIST_HEIGHT) / 2);
	VDP_setScrollingMode(HSCROLL_PLANE, VSCROLL_2TILE);
	VDP_setHInterrupt(1);
	
	s16 defVS[FB_MAX_WIDTH / 16];
	memsetU16((u16 *) defVS, FIRE_DIST_HEIGHT, FB_MAX_WIDTH / 16);
	VDP_setVerticalScrollTile(fb_plan, 0, defVS, FB_MAX_WIDTH / 16, FALSE);
}
void firefx_close(void) {
	FB_close();
	VDP_setHInterrupt(0);
}

void firefx_loop(void) {
	fireBlur();
	drawRandomDots();

	computeWaveScale(waveScaleAlpha++);
	
	FB_upload(FIRE_COMPUTED_WIDTH, FIRE_HEIGHT, 0);
}