#include "genesis.h"
#include "easing_table.h"

#pragma once

#define COL_REDSFT           0
#define COL_GREENSFT         4
#define COL_BLUESFT          8

#define COL_REDMASK          0x000F
#define COL_GREENMASK        0x00F0
#define COL_BLUEMASK         0x0F00

typedef struct
{
    Vect2D_f16 pos;
    Vect2D_f16 mov;
    u16 timer;
} Object;

extern u16 *framerate_adapter;
extern u8 framerate;

#define RSE_FRAMES(A) framerate_adapter[A]
#define RSE_COUNTER(A) (((A) * 60) / framerate)

#define NOINLINE __attribute__((noinline))

const u16 palette_white[64];
const u16 palette_white_bg[64];

void RSE_recalculate_framerate_equivalent(u8 fr);
void RSE_turn_screen_to_white(void);
void RSE_turn_screen_to_black(void);
void RSE_turn_screen_to_color(u16 col);
void RSE_pause(u16 frames);
void RSE_clearTileRowB(u16 row);
void RSE_clearTileRowBWithPrio(u16 row);
void RSE_clearTileRowA(u16 row);
void RSE_clearTileRowAWithPrio(u16 row);
void RSE_resetScrolling(void);
void RSE_clearAll(void);
void RSE_reuploadSystemTiles(void);
void RSE_changeResolution(u16 width);
u16 RSE_colMul(u16 A, u16 B);
u16 RSE_colAdd(u16 A, u16 B);
u16 RSE_colSub(u16 A, u16 B);
u16 RSE_colDivInt(u16 A, u16 B);

void RSE_Safe_SPR_setFrame(Sprite *sprite, s16 frame);

