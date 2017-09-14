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

#define GRAVITY_VELY	FIX32(5.0)
#define GRAVITY_VELY_PAL	FIX32((5.0 * 50.0) / 60.0)
#define BOUNCE_ENERGY_X	FIX32(1.025)
#define BOUNCE_ENERGY_Y	FIX32(0.995)
#define TIME_SCALE 		FIX32(0.075)
#define TIME_SCALE_PAL 	FIX32((0.075 * 60.0) / 50.0)
#define BALL_RADIUS		(112/2)
#define BOOST_FACTOR	FIX32(2.0)

#define RUBR_STRENGTH 768
#define RUBR_DECAY 32

#define GLENZ_SPLIT (112 / 8)

NOINLINE void flat3DGlenzFX(u16 in_phase)
{
	u16 glenz_phase;
	s16 x_glenz, y_glenz;
	u16 glenz_frame;
	u32 prev_music_sync, music_sync = 0;
	Sprite *sprites[GLENZ_SPLIT + 1];
	fix32 velx, vely, x_glenz_f32, y_glenz_f32;
	u8 hit, fading_out = 0;
	u16 pal[64];
	s16 rubr_strength = RUBR_STRENGTH;

	glenz_frame = 0;
	glenz_phase = in_phase;

	SPR_init(GLENZ_SPLIT + 1, 0, 0);
	VDP_setHilightShadow(1);

	VDP_setPalette(PAL2, palette_black);
	VDP_setPalette(PAL3, palette_white);

	for (s16 i = 0; i < GLENZ_SPLIT; i++)
	{
		sprites[i] = SPR_addSprite(&glenz_anim, 0, 0, TILE_ATTR_FULL(PAL3, TRUE, FALSE, FALSE, 0));
		SPR_setAnimAndFrame(sprites[i], i, 0);
	}

	sprites[GLENZ_SPLIT] = SPR_addSprite(&cube_shadow, 0, 0, TILE_ATTR_FULL(PAL3, TRUE, FALSE, FALSE, 0));
	SPR_setAnimAndFrame(sprites[GLENZ_SPLIT], 0, 0);
	//SPR_setFrame(sprites[GLENZ_SPLIT], 0);

	u8 music_checkpoint = 0;

	/* Initial Position */
	x_glenz_f32 = FIX32((sinFix16(((glenz_phase << 1) * 5) >> 1)) + 160 - 64);
	y_glenz_f32 = FIX32(((cosFix16(glenz_phase << 2) * 3) >> 2) + 96 - 64);

	/* Initial Velocity */
	velx = FIX32((sinFix16((((glenz_phase - 10) << 1) * 5) >> 1)) + 160 - 64);
	velx = fix32Sub(velx, x_glenz_f32);
	velx = fix32Mul(velx, BOOST_FACTOR);

	vely = FIX32(((cosFix16((glenz_phase - 10) << 2) * 3) >> 2) + 96 - 64);
	vely = fix32Sub(vely, y_glenz_f32);
	vely = fix32Mul(vely, BOOST_FACTOR);

	// x_glenz_f32 = FIX32(47);
	// y_glenz_f32 = FIX32(-16);
	// velx = FIX32(28);
	// vely = FIX32(0);

	// KLog_S4("x_glenz_f32 = ", fix32ToInt(x_glenz_f32), "y_glenz_f32 = ", fix32ToInt(y_glenz_f32), "velx = ", fix32ToInt(velx), "vely = ", fix32ToInt(vely));

	VDP_interruptFade();
	VDP_fadePalTo(PAL3, glenz_anim.palette->data, RSE_FRAMES(16), TRUE);

	music_sync = music_getElapsed();
	while(music_checkpoint < 3)
	{
		VDP_waitVSync();
		
		DMA_flushQueue();
		if (fading_out)
			fading_out = VDP_doStepFading(0);
		
		prev_music_sync = music_sync;
		music_sync = music_getElapsed();
		updateScrollText();

		if (hit)
		{
			hit = FALSE;
			VDP_interruptFade();
			VDP_fadePalTo(PAL3, glenz_anim.palette->data, RSE_FRAMES(16), TRUE);			
		}

		/* Physics */

		if (framerate == 60)
		{
			vely = fix32Add(vely, GRAVITY_VELY);
			x_glenz_f32 = fix32Add(x_glenz_f32, fix32Mul(TIME_SCALE, velx));
			y_glenz_f32 = fix32Add(y_glenz_f32, fix32Mul(TIME_SCALE, vely));
		}
		else
		{
			vely = fix32Add(vely, GRAVITY_VELY_PAL);
			x_glenz_f32 = fix32Add(x_glenz_f32, fix32Mul(TIME_SCALE_PAL, velx));
			y_glenz_f32 = fix32Add(y_glenz_f32, fix32Mul(TIME_SCALE_PAL, vely));
		}

		/* Bouncing */
		if (x_glenz_f32 > FIX32(320 - BALL_RADIUS * 0.85))
		{
			velx = fix32Mul(BOUNCE_ENERGY_X, fix32Neg(velx));
			x_glenz_f32 = FIX32(320 - BALL_RADIUS * 0.85);
		}
		else
		if (x_glenz_f32 < FIX32(BALL_RADIUS * 0.85))
		{
			velx = fix32Mul(BOUNCE_ENERGY_X, fix32Neg(velx));
			x_glenz_f32 = FIX32(BALL_RADIUS * 0.85);
		}				

		if (y_glenz_f32 > FIX32(224 - BALL_RADIUS + BALL_RADIUS * RUBR_STRENGTH / 4096))
		{
			vely = fix32Mul(BOUNCE_ENERGY_Y, fix32Neg(vely));
			// vely = fix32Neg(vely);
			y_glenz_f32 = FIX32(224 - BALL_RADIUS + BALL_RADIUS * RUBR_STRENGTH / 4096);
			if (music_checkpoint == 0)
			{
				VDP_interruptFade();
				VDP_setPalette(PAL3, palette_white);
				hit = TRUE;
				rubr_strength = 0;
			}
		}
		
		x_glenz = fix32ToInt(x_glenz_f32) - BALL_RADIUS;
		y_glenz = fix32ToInt(y_glenz_f32);

		/* Rubber deformation */
		rubr_strength = min(RUBR_STRENGTH, rubr_strength + RUBR_DECAY * (music_sync - prev_music_sync));
		
		for (s16 i = 0; i < GLENZ_SPLIT; i++)
		{
			s16 pos = i - (GLENZ_SPLIT / 2);
			s16 rubr = 2048 - RUBR_STRENGTH + rubr_strength - (((sinFix16(music_sync << 6) + 64) * rubr_strength) >> 8);
			s16 rubr_pos = (rubr * pos) >> 8;
			SPR_setPosition(sprites[i], x_glenz, y_glenz + rubr_pos);
			
			RSE_Safe_SPR_setFrame(sprites[i], glenz_frame & 127);
		}

		/* Shadow */
		SPR_setPosition(sprites[GLENZ_SPLIT], x_glenz + 32 + ((y_glenz - BALL_RADIUS) >> 3), ((y_glenz - BALL_RADIUS) >> 3) + 190);
		RSE_Safe_SPR_setFrame(sprites[GLENZ_SPLIT], (glenz_frame >> 1) & 63);

		SPR_update(sprites, 2);

		glenz_phase++;
		glenz_frame = glenz_phase >> 1;

		switch(music_checkpoint)
		{
			case 0:
				if (music_sync >= 3980)
					music_checkpoint++;
			break;

			case 1:
				VDP_interruptFade();
				VDP_getPaletteColors(0, pal, 64);
				VDP_initFading(0, 63, pal, palette_black,RSE_FRAMES(32), TRUE);
				fading_out = 1;
				music_checkpoint++;
			break;

			case 2:
				if (!fading_out)
					music_checkpoint++;
			break;
		}
	}

	// RSE_pause(RSE_FRAMES(16));

	VDP_setWindowAddress(0xB000);
	VDP_setWindowHPos(FALSE, 0);
	VDP_setWindowVPos(FALSE, 0);

	RSE_clearAll();
}