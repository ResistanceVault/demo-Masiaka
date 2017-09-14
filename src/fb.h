////////////////////////////////////////////////////////////////////////////////
// Fast Megadrive 4bpp framebuffer emulation
// by GliGli
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <genesis.h>

#define FB_TILES_OFFSET 768 // in tiles

#define FB_MAX_WIDTH 256
#define FB_MAX_HEIGHT 192

extern u16 *fb;
extern VDPPlan fb_plan;
extern s16 fb_width;
extern s16 fb_height;
extern u16 fb_size; // in bytes

#define FB_INIT_INTERLACED 1
#define FB_INIT_FULL_FRAME 2
#define FB_INIT_H40 4

void FB_init(VDPPlan plan, u8 palette, s16 width, s16 height, u16 flags);
void FB_close(void);
void FB_setScroll(s16 x, s16 y);
void FB_upload(s16 width, s16 height, u8 interlaced);
u16 FB_getPlanAddress(void);
