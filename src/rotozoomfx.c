#include "rotozoomfx.h"

#include "fb.h"

static const u8 *rz_tex;

extern u16 innerLoopSMCStart;

inline __attribute__((always_inline))
static u16 makeUV(u16 u, u16 v) {
	u16 ret;

#if 1
	asm volatile (
		"move.w %[u], %[ret]\n"
		"move.w %[v], -(%%sp)\n"
		"move.b (%%sp)+, %[ret]\n"
		: [ret] "=d" (ret)
		: [u] "r" (u), [v] "r" (v)
	);
#else
	ret = (u & 0xff00) | ((v >> 8) & 0x00ff);
#endif		

	return ret;
}

#define DO_4_PX\
	"move.b 0x55aa(%%a0), (%[fbptr])+\n"\
	"move.b 0x55aa(%%a0), (%[fbptr])+\n"

#define DO_32_PX\
	DO_4_PX\
	DO_4_PX\
	DO_4_PX\
	DO_4_PX\
	DO_4_PX\
	DO_4_PX\
	DO_4_PX\
	DO_4_PX

__attribute__((used, noinline, section(".wram_text")))
static void innerLoop(u16 * fbptr, u16 u, u16 v, u16 xx, u16 yy)
{
	u16 uv;
	
	for (s16 x = 0; x < RZ_WIDTH / 8; ++x) {
		uv = makeUV(u, v);

		asm volatile (
			"lea (%[tex], %[uv].w), %%a0\n"

			".global innerLoopSMCStart\n"
			"innerLoopSMCStart:\n"
			DO_32_PX
			DO_32_PX
			DO_32_PX
			DO_32_PX
			DO_32_PX
			DO_32_PX
			DO_32_PX
			DO_32_PX
			DO_32_PX
			DO_32_PX
			DO_32_PX
			DO_32_PX
			DO_32_PX
			DO_32_PX
			DO_32_PX

			: [fbptr] "+a" (fbptr) 
			: [uv] "d" (uv), [tex] "a" (&rz_tex[65536])
			: "a0"
		);

		u -= yy << 3;
		v += xx << 3;
	}
}

void rzfx_init(VDPPlan plan, u8 palette, const u8 *tex) {
	FB_init(plan, palette, RZ_WIDTH, RZ_HEIGHT, FB_INIT_INTERLACED | FB_INIT_FULL_FRAME);
	rz_tex = tex;
}

#define DO_2_SMC(idx)\
	smcPtr[(idx) * 4 + 1] = makeUV(uu2, vv2);\
	smcPtr[(idx) * 4 + 3] = makeUV(uu , vv);\
	uu2 += xx;\
	vv2 += yy;\
	uu += xx;\
	vv += yy;

#define DO_16_SMC(idx)\
	DO_2_SMC((idx) * 8 + 0)\
	DO_2_SMC((idx) * 8 + 1)\
	DO_2_SMC((idx) * 8 + 2)\
	DO_2_SMC((idx) * 8 + 3)\
	DO_2_SMC((idx) * 8 + 4)\
	DO_2_SMC((idx) * 8 + 5)\
	DO_2_SMC((idx) * 8 + 6)\
	DO_2_SMC((idx) * 8 + 7)

void rzfx_loop(s16 cx, s16 cy, fix16 angle, fix16 zoom) {
	u16 xx = fix32Mul(cosFix32(angle), fix16ToFix32(zoom)) >> (FIX32_FRAC_BITS - 8);
	u16 yy = fix32Mul(sinFix32(angle), fix16ToFix32(zoom)) >> (FIX32_FRAC_BITS - 8);
	u16 u, v, uu, vv, uu2, vv2;
	static u16 * smcPtr = &innerLoopSMCStart;	
	
	u = cy << 8;
	v = cx << 8;
	uu = u - xx * RZ_HEIGHT / 2;
	vv = v - yy * RZ_HEIGHT / 2;
	uu2 = uu + (yy << 1);
	vv2 = vv - (xx << 1);
	
	DO_16_SMC( 0)
	DO_16_SMC( 1)
	DO_16_SMC( 2)
	DO_16_SMC( 3)
	DO_16_SMC( 4)
	DO_16_SMC( 5)
	DO_16_SMC( 6)
	DO_16_SMC( 7)
	DO_16_SMC( 8)
	DO_16_SMC( 9)
	DO_16_SMC(10)
	DO_16_SMC(11)
	DO_16_SMC(12)
	DO_16_SMC(13)
	DO_16_SMC(14)

	u += yy * RZ_WIDTH / 2;
	v -= xx * RZ_WIDTH / 2;
	
	innerLoop(fb, u, v, xx, yy);

	u -= yy << 2;
	v += xx << 2;
	
	innerLoop(&fb[RZ_WIDTH * RZ_HEIGHT / 8], u, v, xx, yy);
	
#if 1
	FB_upload(RZ_WIDTH, RZ_HEIGHT, 1);
#else	
	VDP_showFPS(1);
#endif	
}