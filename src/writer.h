#include "genesis.h"

extern u8 letter_width;
extern u16 current_char;
extern VDPPlan current_plan;
extern u8 current_pal;
extern u8 scroll_speed;
extern s16 scroll_x_offset;
extern u8 scroll_local_offset;
extern const char *demo_strings;

#ifndef RSE_WRITER
#define RSE_WRITER

u16 RSE_writerSetup(void);
void RSE_writerSetY(u16 initial_y);
void updateScrollText(void);

#endif
