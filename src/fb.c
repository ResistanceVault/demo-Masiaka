#include "fb.h"

u16 *fb;
VDPPlan fb_plan;
s16 fb_width;
s16 fb_height;
u16 fb_size; // in bytes

static void * fb_rawptr;
static _voidCallback *fb_hIntSave;
static u8 fb_fullFB;
static vu8 fb_readyToUpload;
static vu8 fb_hIntState;

static void FB_preparePlan(u8 palette, u8 interlaced) {
	u16 i, j, pw, sz;
	u16* buf;
	
	pw = VDP_getPlanWidth();
	sz = pw * (FB_MAX_HEIGHT / 8);
	
	buf = (u16*)MEM_alloc(sz << 1);
	memsetU16(buf, 0, sz);
	
	if (interlaced) {
		for (j = 0; j < (fb_height >> 3); ++j) {
			for (i = 0; i < (fb_width >> 3); ++i) {
				buf[pw * j + i] = TILE_ATTR_FULL(palette, 0, 0, 0, FB_TILES_OFFSET + (fb_height >> 3) * i + j);
			}
		}
	} else {
		for (j = 0; j < (fb_height >> 3); ++j) {
			for (i = 0; i < (fb_width >> 3); ++i) {
				buf[pw * j + i] = TILE_ATTR_FULL(palette, 0, 0, 0, FB_TILES_OFFSET + (FB_MAX_HEIGHT / 8) * i + j);
			}
		}
	}
	
	VDP_setTileMapData(FB_getPlanAddress(), buf, 0, sz, TRUE);
	
	MEM_free(buf);
}

void FB_vInt_fullFB(void) {
	fb_readyToUpload = 1;
}


void FB_hInt(void) {
	fb_readyToUpload = 1;
}

u16 FB_getPlanAddress(void) {
	switch (fb_plan.value) {
	case CONST_PLAN_A:
		return VDP_getAPlanAddress();
		break;
	case CONST_PLAN_B:
		return VDP_getBPlanAddress();
		break;
	case CONST_PLAN_WINDOW:
		return VDP_getWindowPlanAddress();
		break;
	}
	return 0;
}

void FB_init(VDPPlan plan, u8 palette, s16 width, s16 height, u16 flags) {
	fb_plan = plan;
	u8 en = VDP_getEnable();
	
	VDP_setEnable(0); // for faster DMA
	
	fb_fullFB = flags & FB_INIT_FULL_FRAME; 
	fb_width = width;
	fb_height = height;
	fb_size = (width  * height) >> 1;
	
	fb_rawptr = MEM_alloc(fb_size + 256);
	fb = (u16 *)(((u32)fb_rawptr + 4) & ~3);
	
	if (flags & FB_INIT_H40)
		VDP_setScreenWidth320();
	else
		VDP_setScreenWidth256();

	VDP_fillTileData(0, FB_TILES_OFFSET, width * FB_MAX_HEIGHT / (TILE_SIZE * 2), FALSE); // clear framebuffer in VRAM
	memsetU32((u32*) fb, 0, fb_size >> 2); // clear framebuffer in WRAM
	DMA_waitCompletion();
	
	FB_setScroll(0, 0);
	FB_preparePlan(palette, flags & FB_INIT_INTERLACED);	
	
	VDP_setHInterrupt(0);

	if (fb_fullFB) {
		SYS_setVIntCallback(FB_vInt_fullFB);
		fb_hIntSave = internalHIntCB;
	} else {
		VDP_setHIntCounter((screenHeight + fb_height) >> 1);
		fb_hIntSave = internalHIntCB;
		internalHIntCB = FB_hInt;
		VDP_setHInterrupt(1);
	}

	VDP_setEnable(en);
}

void FB_close(void) {
	SYS_setVIntCallback(NULL);
	internalHIntCB = fb_hIntSave;
	MEM_free(fb_rawptr);
	fb = NULL;
}

void FB_setScroll(s16 x, s16 y) {
	VDP_setHorizontalScroll(fb_plan, x);
	
	if (fb_fullFB) {
		VDP_setVerticalScroll(fb_plan, y);
	} else {
		VDP_setVerticalScroll(fb_plan, y + ((fb_height - screenHeight) >> 1));
	}
}


#define DO_DMA_8_COLS(w, x)\
	if (w > (x) * 8) {\
		*pw = lenreg;\
		*pl = GFX_DMA_VRAM_ADDR(FB_TILES_OFFSET * TILE_SIZE + (x) * 4 * FB_MAX_HEIGHT);\
		*pw = lenreg;\
		*pl = GFX_DMA_VRAM_ADDR(FB_TILES_OFFSET * TILE_SIZE + (x) * 4 * FB_MAX_HEIGHT + 2);\
	}
	
#define DO_DMA_32_COLS(w, x)\
	DO_DMA_8_COLS(w, (x) * 4 + 0)\
	DO_DMA_8_COLS(w, (x) * 4 + 1)\
	DO_DMA_8_COLS(w, (x) * 4 + 2)\
	DO_DMA_8_COLS(w, (x) * 4 + 3)
	
inline __attribute__((always_inline))
void FB_upload(s16 width, s16 height, u8 interlaced) {
	u32 from;
	register u32 lenreg asm ("d0");
	vu16 *pw = (u16 *) GFX_CTRL_PORT;
    vu32 *pl = (u32 *) GFX_CTRL_PORT;
	u8 reg1Save = VDP_getReg(1);

	if (interlaced) {
		lenreg = (width * height) >> 3;
		lenreg = 0x94009300 | (lenreg & 0xff) | ((lenreg & 0xff00) << 8);
	} else {
		lenreg = 0x9300 | height;
	}
			
	from = (u32)fb >> 1;
	
	fb_readyToUpload = 0;
	while (!fb_readyToUpload);

	VDP_setAutoInc(4);
	*pw = 0x8134; // disable display (for faster DMA no matter the VBlank state)

    *pw = 0x9500 + (from & 0xff);
    from >>= 8;
    *pw = 0x9600 + (from & 0xff);
    from >>= 8;
    *pw = 0x9700 + (from & 0x7f);
	
	if(interlaced) {
		*pl = lenreg;
		*pl = GFX_DMA_VRAM_ADDR(FB_TILES_OFFSET * TILE_SIZE);
		*pl = lenreg;
		*pl = GFX_DMA_VRAM_ADDR(FB_TILES_OFFSET * TILE_SIZE + 2);
	} else {
		DO_DMA_32_COLS(width, 0)
		DO_DMA_32_COLS(width, 1)
		DO_DMA_32_COLS(width, 2)
		DO_DMA_32_COLS(width, 3)
		DO_DMA_32_COLS(width, 4)
		DO_DMA_32_COLS(width, 5)
		DO_DMA_32_COLS(width, 6)
		DO_DMA_32_COLS(width, 7)
	}

	*pw = 0x8100 | reg1Save;
}
