#include "genesis.h"
#include <gfx.h>
#include "writer.h"
#include "transition_helper.h"
#include "string_parser.h"

extern u16 vramIndex;
extern u8 framerate;
const char *demo_strings;

/* 
	Global writer data 
*/

u16 current_char_x;
u16 current_char_y;
u8 letter_width;
u16 current_char;
VDPPlan current_plan;
u8 current_pal;
u8 scroll_speed;
s16 scroll_x_offset;
u16 scroll_y_offset;
u8 scroll_local_offset;
u8 screen_w_tile;

#define M_PI 3.14159265358979323846
#define sinFix16Const(x) FIX16(__builtin_sin((x) / 512.0 * M_PI))

#define stval(x) (((sinFix16Const((x) << 4) + sinFix16Const((x) << 2) + 64) * 3) >> 3) - 48

#define stval32(x)\
	stval((x) * 32 +  0), stval((x) * 32 +  1), stval((x) * 32 +  2), stval((x) * 32 +  3),\
	stval((x) * 32 +  4), stval((x) * 32 +  5), stval((x) * 32 +  6), stval((x) * 32 +  7),\
	stval((x) * 32 +  8), stval((x) * 32 +  9), stval((x) * 32 + 10), stval((x) * 32 + 11),\
	stval((x) * 32 + 12), stval((x) * 32 + 13), stval((x) * 32 + 14), stval((x) * 32 + 15),\
	stval((x) * 32 + 16), stval((x) * 32 + 17), stval((x) * 32 + 18), stval((x) * 32 + 19),\
	stval((x) * 32 + 20), stval((x) * 32 + 21), stval((x) * 32 + 22), stval((x) * 32 + 23),\
	stval((x) * 32 + 24), stval((x) * 32 + 25), stval((x) * 32 + 26), stval((x) * 32 + 27),\
	stval((x) * 32 + 28), stval((x) * 32 + 29), stval((x) * 32 + 30), stval((x) * 32 + 31),

const s16 scroll_tile_y[1024] = {
	stval32( 0) stval32( 1) stval32( 2) stval32( 3)
	stval32( 4) stval32( 5) stval32( 6) stval32( 7)
	stval32( 8) stval32( 9) stval32(10) stval32(11)
	stval32(12) stval32(13) stval32(14) stval32(15)
	stval32(16) stval32(17) stval32(18) stval32(19)
	stval32(20) stval32(21) stval32(22) stval32(23)
	stval32(24) stval32(25) stval32(26) stval32(27)
	stval32(28) stval32(29) stval32(30) stval32(31)
};

u16 RSE_writerSetup(void)
{
	current_plan = PLAN_A;
	current_pal = PAL0;

	VDP_loadTileSet(sim1_font.tileset, vramIndex, TRUE);
	vramIndex += sim1_font.tileset->numTile;

	current_char = 0;
	letter_width = 8;
	scroll_speed = 1;
	scroll_x_offset = 0;
	scroll_y_offset = 0;
	scroll_local_offset = 0;
	current_char_x = 0;
	current_char_y = 3;
	screen_w_tile = 64;

	return vramIndex;
}
// s16 scroll_tile_x[16];
void updateScrollText(void)
{
	scroll_x_offset -= scroll_speed;
	scroll_y_offset += scroll_speed;
	scroll_local_offset += scroll_speed;

	current_char_x = ((-scroll_x_offset) >> 3) - 1;

	if (scroll_local_offset >= letter_width)
	{
		char c;

		scroll_local_offset = 0;
		c = demo_strings[current_char];

		if (c == '\0')
			current_char = 1;
		else
		{
			if (c != ' ')
				VDP_setTileMapXY(current_plan, TILE_ATTR_FULL(current_pal, TRUE, FALSE, FALSE, TILE_USERINDEX + charToTileIndex(c)), current_char_x, current_char_y); //TILE_ATTR_FULL(current_pal, FALSE, FALSE, FALSE, current_char_y));
			else
				VDP_setTileMapXY(current_plan, 0, current_char_x, current_char_y); //TILE_ATTR_FULL(current_pal, FALSE, FALSE, FALSE, current_char_y));

		}

		if (scroll_x_offset <= (screen_w_tile * -8))
			scroll_x_offset -= screen_w_tile * -8;

		current_char++;
	}

	SYS_disableInts();

	/* V scroll */
	VDP_setVerticalScrollTile(current_plan, 0, (s16*)&(scroll_tile_y[scroll_y_offset & 0x1ff]), (screen_w_tile >> 1) - 4, FALSE);
	/* H scroll */
	VDP_setHorizontalScroll(current_plan, scroll_x_offset);

	SYS_enableInts();
}

void RSE_writerSetY(u16 initial_y)
{	current_char_y = initial_y;	}