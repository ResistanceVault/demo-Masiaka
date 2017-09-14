#pragma once

#include "genesis.h"

#define FONT_PUNCT_OFFSET 36
#define FONT_LINE_OFFSET ((432 >> 3))

u16 computeStringLen(char *str);
u16 charToTileIndex(char c);