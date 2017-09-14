#pragma once

#include "genesis.h"

extern u16 pgwriter_state;
extern u16 pgwriter_switch;
extern u16 pg_current_string_idx;
extern u16 pg_current_string_len;
extern u16 pg_current_char_idx;
extern VDPPlan pg_current_plan;
extern u16 pg_current_char_x;
extern u16 pg_initial_char_y;
extern u16 pg_current_char_y;
extern u16 pgwriter_timer;
extern u16 pgwriter_options;
extern u16 pgwriter_display_duration;
extern u16 pg_x_offset;
extern u8 pgwriter_clear_prev_tile;
extern char **current_strings;

#ifndef RSE_PGWRITER
#define RSE_PGWRITER

/*
	States of the text pgwriter
*/
#define WRT_CENTER_CUR_LINE 0
#define WRT_WRITE_CUR_LINE 1
#define WRT_WAIT 2
#define WRT_CLEAR_LINE 3
#define WRT_IDLE_MODE 16

/*
	Writer options
	/!\ NOT IMPLEMENTED YET
*/
// #define WRT_OPT_WRITE_TO_PLAN_A		1
#define WRT_OPT_AUTO_LINE_FEED 		(1 << 1)
#define WRT_OPT_AUTO_NEXT_STRING	(1 << 2)
#define WRT_OPT_AUTO_RESTART		(1 << 3)
#define WRT_OPT_HALF_SPEED			(1 << 4)

#define WRT_HAS_OPTION(OPT) (OPT & pgwriter_options)

u16 RSE_pgwriterSetup(void);
void RSE_pgwriterSetInitialY(u16 initial_y);
void RSE_pgwriterSetDisplayDuration(u8 duration);
void RSE_pgwriterRestart(void);
u8 RSE_pgwriterIsDone(void);
u16 RSE_pgwriterDrawString(char *str);
void RSE_pgwriterUpdateLine(void);
void RSE_pgwriterUpdateMultiLine(void);
u16 RSE_pgwriterSetOption(u16 option);
u16 RSE_pgwriterUnsetOption(u16 option);
void RSE_pgwriterSetXOffset(u16 offset);

#endif
