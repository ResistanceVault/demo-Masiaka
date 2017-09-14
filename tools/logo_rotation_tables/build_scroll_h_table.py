import os
import string
import codecs
import ast
import math

filename_out			=	"../../src/scroll_h_table"
SCR_TILE_H				=	(100 >> 3)
SCR_SCROLL_STEP_H 		=	8
MODE7_AMPLITUDE			=	1
table_size				=	SCR_TILE_H * SCR_SCROLL_STEP_H
scroll_h 				=	[None] * table_size

def Clamp(k, a, b):
	k = min(k, b)
	k = max(k, a)
	return k


def	EaseInOutQuick(x):
	x = Clamp(x, 0.0, 1.0)
	return	(x * x * (3 - 2 * x))


def EaseInOutByPow(x, p = 2.0):
	x = Clamp(x, 0.0, 1.0)
	y = math.pow(x, p) / (math.pow(x, p) + math.pow(1 - x, p))


def premul_fun(x):
	global scroll_h
	return scroll_h[x]


def dumpFunction(_precalc_func, display_name, f):
	f.write('const s16 ' + display_name + '[] =' + '\n')
	f.write('{' + '\n')

	cr = 0
	for angle in range(table_size):
		_value = _precalc_func(angle)
		_str_out = str(_value) + ','
		if cr > 16:
			_str_out += '\n'
			cr = 0
		f.write('\t' + _str_out)
		cr += 1

	if cr != 0:
		f.write('\n')		

	f.write('};' + '\n')


def  main():
	##	Creates the header
	f = codecs.open(filename_out + '.h', 'w')

	f.write('#pragma once\n\n')

	f.write('#include <genesis.h>\n\n')
	f.write('#define LOGO_TRAJ_LEN_H ' + str(table_size) + '\n')
	f.write('\n')

	f.write('const s16 scroll_h[LOGO_TRAJ_LEN_H];' + '\n')

	f.close()

	##	Creates the C file
	f = codecs.open(filename_out + '.c', 'w')

	f.write('#include <genesis.h>\n\n')

	# for j in range(SCR_SCROLL_STEP_W):
	# 	for i in range(SCR_TILE_W):
	# 		right_pt = (int(SCR_SCROLL_STEP_W - j)) << MODE7_AMPLITUDE
	# 		left_pt = int(j) << MODE7_AMPLITUDE
	# 		scroll_h[i + (j * SCR_TILE_W)] = (right_pt * i) // SCR_TILE_W + (left_pt * (SCR_TILE_W - i)) // SCR_TILE_W

	for j in range(SCR_SCROLL_STEP_H):
		for i in range(SCR_TILE_H):
			right_pt = (int(SCR_SCROLL_STEP_H - j)) << MODE7_AMPLITUDE
			left_pt = int(j) << MODE7_AMPLITUDE
			scroll_h[i + (j * SCR_TILE_H)] = (right_pt * i) // SCR_TILE_H + (left_pt * (SCR_TILE_H - i)) // SCR_TILE_H

	dumpFunction(_precalc_func = premul_fun, display_name = 'scroll_h', f = f)

	f.write('\n')

	f.close()

main()