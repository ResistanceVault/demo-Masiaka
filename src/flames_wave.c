#include <genesis.h>
#include <gfx.h>
#include "transition_helper.h"
#include "logo_yscroll_table.h"
#include "twister_jump_table.h"

#define TWISTER_TABLE_SIZE 1024
#define CST_CTRL_PORT 0xC00004
#define CST_DATA_PORT 0xC00000
#define CST_WRITE_VSRAM_ADDR(adr)   ((0x4000 + ((adr) & 0x3FFF)) << 16) + (((adr) >> 14) | 0x10)

extern u16 vramIndex;
// extern u16 fontIndex;
extern u8 framerate;

static 	Sprite *sprites[16];
static u16 tmp_vcount;

NOINLINE void flamesWavesFX(void)
{
	_voidCallback *hIntSave = internalHIntCB;
	u16 vramIndex = TILE_USERINDEX;
	s16 i, j, k;
	s16 r, g, b;
	u32 music_sync = 0;
	u16 col;
	u16 vcount = 0;
	u16 *pal_raster;

    vu16 *pwd;
    vu32 *plc, *pld;

    pld = (u32 *) GFX_DATA_PORT;
    pwd = (u16 *) GFX_DATA_PORT;
    plc = (u32 *) GFX_CTRL_PORT;

	auto void hBlank()
	{
		tmp_vcount = (GET_VCOUNTER + vcount) & 1023;
	    i = tmp_vcount & 0x7C;
		
	    /* Colors */
		u32 * prl = (u32*)&pal_raster[i + k];
		VDP_setAutoInc(2);
		*plc = GFX_WRITE_CRAM_ADDR(0);
		*pld = prl[0];
		*pld = prl[1];
		
		/* Vertical scroll */
	    *plc = CST_WRITE_VSRAM_ADDR(0);
	    *pwd = twister_jump_table[tmp_vcount];

		/* Horizontal scroll */
	    *plc = GFX_WRITE_VRAM_ADDR(0xB800);
	    *pwd = twister_hjump_table[tmp_vcount];    
	}

	auto void prepareRasterPalette(void)
	{
		/* prepare raster palettes */
		for(k = 0; k < 14; k++)
		{
			// VDP_waitVSync();
			for(i = 0; i < 32; i++)
				for(j = 0; j < 4; j++)
				{
					col = flames_0.palette->data[j];
					r = (col & (0xF << 8)) >> 8;
					g = (col & (0xF << 4)) >> 4;
					b = col & 0xF;

					if (i < 16)
					{
						r -= i;
						g -= (i >> 1);
						b -= (i >> 2);
					}
					else
					{
						r = r - (32 - i);				
						g = g - ((32 - i) >> 1);
						b = b - ((32 - i) >> 2);
					}

					r -= (k + 1) << 1;
					g -= (k + 1) << 1;
					b -= (k + 1) << 1;

					if (r < 0) r = 0;
					if (g < 0) g = 0;
					if (b < 0) b = 0;

					col = (r << 8) | (g << 4) | b;

					pal_raster[(i * 4) + j + (k * 4 * 32)] = col;
				}	
		}		
	}
	
	pal_raster = MEM_alloc(4 * 32 * 14 * sizeof(u16));

	RSE_changeResolution(320);

	// SYS_disableInts();

	/* Set a larger tileplan to be able to scroll */
	VDP_setPlanSize(64, 32);

	SPR_init(0, 0, 0);

	VDP_clearPlan(PLAN_A, TRUE);
	VDP_clearPlan(PLAN_B, TRUE);
	VDP_setScrollingMode(HSCROLL_PLANE, VSCROLL_PLANE);
	VDP_setHInterrupt(TRUE);
	VDP_setHIntCounter(1);

	vramIndex = 8;
	VDP_setEnable(0);

	/* Load the fond tiles */
	VDP_drawImageEx(PLAN_A, &flames_0, TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, vramIndex), 0, 0, FALSE, FALSE);

	VDP_drawImageEx(PLAN_A, &flames_0, TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, vramIndex), 0, 256 >> 3, FALSE, FALSE);
	vramIndex += flames_0.tileset->numTile;;

	VDP_drawImageEx(PLAN_A, &flames_1, TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, vramIndex), 10, 0, FALSE, FALSE);

	VDP_drawImageEx(PLAN_A, &flames_1, TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, vramIndex), 10, 256 >> 3, FALSE, FALSE);
	vramIndex += flames_1.tileset->numTile;

	VDP_drawImageEx(PLAN_A, &flames_2, TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, vramIndex), 20, 0, FALSE, FALSE);

	VDP_drawImageEx(PLAN_A, &flames_2, TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, vramIndex), 20, 256 >> 3, FALSE, FALSE);
	vramIndex += flames_2.tileset->numTile;

	VDP_drawImageEx(PLAN_A, &flames_3, TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, vramIndex), 30, 0, FALSE, FALSE);

	VDP_drawImageEx(PLAN_A, &flames_3, TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, vramIndex), 30, 256 >> 3, FALSE, FALSE);
	vramIndex += flames_3.tileset->numTile;

	VDP_setVerticalScroll(PLAN_B, (easing_table[64 << 4] >> 4) - 64 - 64);
	VDP_drawImageEx(PLAN_B, &masiaka_title_pic, TILE_ATTR_FULL(PAL1, TRUE, FALSE, FALSE, vramIndex), (320 - 240) >> 4, (224 - 48) >> 4, FALSE, TRUE);
	vramIndex += masiaka_title_pic.tileset->numTile;

	prepareRasterPalette();	

	VDP_setEnable(1);

	// sprites[0] = SPR_addSprite(&sword, (320 - 48) >> 1, -128, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, vramIndex));
	sprites[0] = SPR_addSpriteEx(&sword, (320 - 48) >> 1, -128, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, vramIndex), 0, SPR_FLAG_AUTO_VISIBILITY | !SPR_FLAG_AUTO_VRAM_ALLOC | SPR_FLAG_AUTO_SPRITE_ALLOC | SPR_FLAG_AUTO_TILE_UPLOAD);
	SPR_update(sprites, 1);
	DMA_flushQueue();

    VDP_setHilightShadow(0);

	vcount = 0;
	VDP_fadePalTo(PAL3, sword.palette->data, RSE_FRAMES(8), TRUE);
    while (vcount < 32)	
	{
		VDP_waitVSync();
    	SPR_setPosition(sprites[0], (320 - 48) >> 1, (((224 - 128) >> 1) - 76 )+ (easing_table[vcount << 5] >> 4));
    	SPR_update(sprites, 1);
		DMA_flushQueue();
    	vcount++;
	}

	// music_sync = music_getElapsed();
	// KLog_U2("flames_wave.c, music_sync = ", music_sync, ", vcount = ", vcount);
	while(music_sync < 9750)
	{
		VDP_waitVSync();
		music_sync = music_getElapsed();
	}
	// music_sync = music_getElapsed();
	// KLog_U2("flames_wave.c, music_sync = ", music_sync, ", vcount = ", vcount);	

	j = 0;
	k = 0;
	vcount = 0;

    while (vcount < 60 * 10)
    {
        VDP_waitVSync();

        switch(vcount)
        {
        	case 0:
	        	VDP_fadeTo(16, 31, palette_white, 6, TRUE);
	        	break;        	
        	case 2 + 8:
	        	VDP_fadePalTo(PAL1, masiaka_title_pic.palette->data, RSE_FRAMES(16), TRUE);
	        	break;
        	case 10 + 8:
				VDP_setHInterrupt(1);
				internalHIntCB = hBlank;
	        	break;
        	case 60 * 10 - 20:
	        	VDP_fadePalTo(PAL1, palette_black, RSE_FRAMES(16), TRUE);
	        	break;
		}

        if (vcount < 64)
        {
		    *plc = GFX_WRITE_VSRAM_ADDR(2);
		    *pwd = logo_yscroll_table[vcount];        	
        }


        *plc = GFX_WRITE_VSRAM_ADDR(0);
	    *pwd = twister_jump_table[vcount & 1023];

	    *plc = GFX_WRITE_VRAM_ADDR(0xB800);
	    *pwd = twister_hjump_table[vcount & 1023];	 	    
        
        if (vcount & 0x1 && vcount > 60 * 5 && k < (4 * 13 * 32))
        	k += 8;

        vcount += 1;
    }

	internalHIntCB = hIntSave;
	
	MEM_free(pal_raster);
	
	RSE_clearAll();

	// music_sync = music_getElapsed();
	// KLog_U2("flames_wave.c, music_sync = ", music_sync, ", vcount = ", vcount);	
}

