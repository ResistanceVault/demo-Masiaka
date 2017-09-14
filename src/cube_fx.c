#include <genesis.h>
#include <gfx.h>
#include "ball_coords.h"
#include "quicksort.h"
#include "writer.h"
#include "transition_helper.h"
#include "music.h"

extern u16 vramIndex;
// extern u16 fontIndex;
extern u8 framerate;

#define CUBE_SPLIT (112 / 8)

NOINLINE u16 flat3DCubeFX(void)
{
	u16 i, cube_phase, cube_phase_pal;
	s16 x_cube, y_cube;
	u16 cube_frame;
	u32 music_sync = 0;
	Sprite *sprites[CUBE_SPLIT + 1];

	cube_frame = 0;
	cube_phase = 256;

	SPR_init(CUBE_SPLIT + 1, 0, 0);
	VDP_setHilightShadow(1);

	VDP_setPalette(PAL2, palette_black);

	for (i = 0; i < CUBE_SPLIT; i++)
	{
		sprites[i] = SPR_addSprite(&cube_anim, 0, 0, TILE_ATTR_FULL(PAL2, TRUE, FALSE, FALSE, 0));
		SPR_setFrame(sprites[i], 0);
		SPR_setAnim(sprites[i], i);
	}

	sprites[CUBE_SPLIT] = SPR_addSprite(&cube_shadow, 0, 0, TILE_ATTR_FULL(PAL3, TRUE, FALSE, FALSE, 0));
	SPR_setFrame(sprites[CUBE_SPLIT], 0);

	u8 music_checkpoint = 0;

	while(music_checkpoint < 10) // music_sync < 3200)
	{
		VDP_waitVSync();

		DMA_flushQueue();
		music_sync = music_getElapsed();
		updateScrollText();	

		cube_phase_pal = (cube_phase * 1155) / 1006; /* Magic formula */

		if (music_sync > 2470)
		{
			if (framerate == 60)
			{
				x_cube = (sinFix16(((cube_phase << 1) * 5) >> 1)) + 160 - 64;
				y_cube = ((cosFix16(cube_phase << 2) * 3) >> 2) + 96 - 64;
			}
			else
			{
				x_cube = (sinFix16(((cube_phase_pal << 1) * 5) >> 1)) + 160 - 64;
				y_cube = ((cosFix16(cube_phase_pal << 2) * 3) >> 2) + 96 - 64;
			}

			if (music_sync < 2880)
			{
				for (i = 0; i < CUBE_SPLIT; i++)
				{
					SPR_setPosition(sprites[i], x_cube, y_cube + (i << 3));
					RSE_Safe_SPR_setFrame(sprites[i], cube_frame & 127);
				}
			}
			else
			{
				for (i = 0; i < CUBE_SPLIT; i++)
				{
					SPR_setPosition(sprites[i], x_cube, y_cube + (i << 3));
					RSE_Safe_SPR_setFrame(sprites[i], (cube_frame + i) & 127);
				}
			}

			SPR_setPosition(sprites[CUBE_SPLIT], x_cube + 32 + (y_cube >> 3), (y_cube >> 3) + 190);
			RSE_Safe_SPR_setFrame(sprites[CUBE_SPLIT], (cube_frame >> 1) & 63);

			SPR_update(sprites, CUBE_SPLIT + 1);

			cube_phase++;
			cube_frame = cube_phase >> 1;

			if (cube_phase == RSE_FRAMES(256 + 32))
				VDP_fadePalTo(PAL2, palette_white, RSE_FRAMES(4), TRUE);
			else
			if (cube_phase == RSE_FRAMES(256 + 32 + 10))
				VDP_fadePalTo(PAL2, sky.palette->data, RSE_FRAMES(16), TRUE);

			switch(music_checkpoint)
			{
				case 0:
					if (music_sync >= 2470)
					{
						VDP_interruptFade();
						VDP_fadePalTo(PAL2, palette_white, RSE_FRAMES(4), TRUE);
						music_checkpoint++;
					}
					break;

				case 1:
					if (music_sync >= 2470 + 25)
					{
						VDP_interruptFade();
						VDP_fadePalTo(PAL2, sky.palette->data, RSE_FRAMES(16), TRUE);
						music_checkpoint++;
					}
					break;

				case 2:
					if (music_sync >= 2700)
					{
						VDP_interruptFade();
						VDP_fadePalTo(PAL2, palette_white, RSE_FRAMES(4), TRUE);
						music_checkpoint++;
					}
					break;

				case 3:
					if (music_sync >= 2700 + 25)
					{
						VDP_interruptFade();
						VDP_fadePalTo(PAL2, sky.palette->data, RSE_FRAMES(16), TRUE);			
						music_checkpoint++;
					}
					break;

				case 4:
					if (music_sync >= 2880)
					{
						VDP_interruptFade();
						VDP_fadePalTo(PAL2, palette_white, RSE_FRAMES(4), TRUE);
						music_checkpoint++;
					}
					break;

				case 5:
					if (music_sync >= 2880 + 25)
					{
						VDP_interruptFade();
						VDP_fadePalTo(PAL2, sky.palette->data, RSE_FRAMES(16), TRUE);
						music_checkpoint++;
					}
					break;

				case 6:
					if (music_sync >= 3080)
					{
						VDP_interruptFade();
						VDP_fadePalTo(PAL2, palette_white, RSE_FRAMES(4), TRUE);
						music_checkpoint++;
					}
					break;

				case 7:
					if (music_sync >= 3080 + 25)
					{
						VDP_interruptFade();
						VDP_fadePalTo(PAL2, sky.palette->data, RSE_FRAMES(16), TRUE);
						music_checkpoint++;
					}
					break;

				case 8:
					if (music_sync >= 3220)
					{
						VDP_interruptFade();
						VDP_fadePalTo(PAL2, palette_white, RSE_FRAMES(24), TRUE);
						music_checkpoint++;
					}
					break;

				case 9:
					if (!VDP_isDoingFade())
						music_checkpoint++;
					break;						
			}
		}
	}

	// RSE_pause(RSE_FRAMES(16));
	SPR_end();
	VDP_waitVSync();
	updateScrollText();

	// KLog_S1("cube_phase = ", (s32)cube_phase);
	// KLog_S1("cube_phase_pal = ", (s32)cube_phase_pal);
	// // NTSC cube_phase = 1155
	// // PAL : cube_phase = 1006
	// // PAL : cube_phase_pal = 1206
	if (framerate == 60)
		return cube_phase;
	else
		return cube_phase_pal;
}