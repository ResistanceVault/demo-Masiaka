#include "music.h"
#include "resources.h"

extern u8 framerate;
vu32 music_elapsed = 0;

static void music_vInt(void)
{
	music_elapsed = XGM_getElapsed();
}

void play_intro_sound(void)
{
	stop_music();
	SND_startPlay_XGM(logo_boot_music);
	SND_setMusicTempo_XGM(50);
	SYS_setVIntPreCallback(music_vInt);
}

void play_music(void)
{
	stop_music();
	SND_startPlay_XGM(vgm_music);
	SND_setMusicTempo_XGM(50);
	SYS_setVIntPreCallback(music_vInt);
}

void wait_until_music_sync(u32 sync_value)
{
	while(TRUE)
	{
		VDP_waitVSync();
		if (music_elapsed > sync_value)
			return;
	}
}

// purpose of this function is to prevent calling XGM_getElapsed too often
// so as not to interfere with the Z80 music player
u32 music_getElapsed(void)
{
	return music_elapsed;
}

void benchmark_music_timings(const u8 *song)
{
	u8 music_played = FALSE;
	u32 vcount = 0, sync = 0, start_time = 0, time = 0;
	char str[64];

	VDP_drawText("XGM Elapsed time : ", 1, 1);
	VDP_drawText("Video freq : ", 1, 2);
	intToStr(framerate, str, 1);
	VDP_drawText(str, 14, 2);

	start_time = getTime(getTick());

	while(TRUE) // sync < 1000)
	{
		VDP_waitVSync();

		if (vcount > (framerate << 1))
		{
			if (!music_played)
			{
				SND_startPlay_XGM(song);
				SND_setMusicTempo_XGM(50);
				music_played = TRUE;
				start_time = getTime(getTick());
			}
			time = getTime(getTick()) - start_time;
		}

		sync = XGM_getElapsed();
		intToStr(sync, str, 1);
		VDP_drawText(str, 1, 3);
		intToStr(time, str, 1);
		VDP_drawText(str, 1, 4);		
		vcount++;
	}

	while(TRUE)
		VDP_waitVSync();

}

void stop_music(void)
{	
	SYS_setVIntPreCallback(NULL);
	SND_stopPlay_XGM();
}
