#include "genesis.h"
#include <gfx.h>
#include <resources.h>
#include "writer.h"
#include "transition_helper.h"
#include "music.h"
#include "db.h"

/* 
Resistance 2016

Code : Fra
Music : Nainain
Gfx : ...
Framework : SGDK
*/

void fastVectorBallFX(Sprite **);
void main_logo(void);
void shieldAnimFX(void);
u16 flat3DCubeFX(void);
void flat3DGlenzFX(u16);
Sprite **displayBarbTitleFX(void);
void displayBarbPictureFX(void);
void flamesWavesFX(void);
void ankouScreenFX(void);
void RSE_Starfield_3D_Spr(void);
void fireFX(void);
void rotoFX(void);
void raytraceFX(void);
void owlFX(void);

u16 vramIndex;
// u16 fontIndex;
u8 framerate;
// char *demo_strings;

int main()
{
	u32 start_time = 0;
	u16 passing_phase;
	Sprite **passing_sprites;

	auto void wait_until_time(u32 time_value)
	{
		VDP_waitVSync();

		while(TRUE)
			if (getTime(getTick()) - start_time > time_value)
				return;
	}

	framerate = 60;
	if (SYS_isPAL())
		framerate = 50;

	RSE_recalculate_framerate_equivalent(framerate);
	DMA_setAutoFlush(FALSE);
	DMA_setMaxTransferSize(0);

	getDbStarts();
	checkDbs(0, 0);

	while(TRUE)
	{
		RSE_clearAll();
		start_time = getTime(getTick());

		main_logo();
		wait_until_time(4300);
		
		// RSE_turn_screen_to_color(0xF0F);
		// RSE_pause(RSE_FRAMES(16));
		// RSE_turn_screen_to_color(0x000);

		play_music();

		passing_sprites = displayBarbTitleFX();

		wait_until_music_sync(515);

		fastVectorBallFX(passing_sprites);
		passing_phase = flat3DCubeFX();
		flat3DGlenzFX(passing_phase);

		wait_until_music_sync(4040);

		fireFX();

		shieldAnimFX();

		raytraceFX();

		ankouScreenFX();

		rotoFX();

		owlFX();

		flamesWavesFX();

		displayBarbPictureFX();

		RSE_Starfield_3D_Spr();

		stop_music();

		RSE_pause(RSE_FRAMES(20));
	}
}
