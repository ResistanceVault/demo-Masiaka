#pragma once

#include <genesis.h>

#define RZ_TEX_SIZE 256
#define RZ_WIDTH 168
#define RZ_HEIGHT 120

void rzfx_init(VDPPlan plan, u8 palette, const u8 *tex);
void rzfx_loop(s16 cx, s16 cy, fix16 angle, fix16 zoom);