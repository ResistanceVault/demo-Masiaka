#include "genesis.h"

void play_intro_sound(void);
void play_music(void);
void stop_music(void);
void benchmark_music_timings(const u8 *song);
void wait_until_music_sync(u32 sync_value);
u32 music_getElapsed(void);
