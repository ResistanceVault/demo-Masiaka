import os
import string
import codecs
import ast
import math

filename_out			=	"../../src/twister_jump_table"
table_size				=	1024

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

def sinFix16(x):
	x = math.sin(x * math.pi * 2 / table_size)
	x *= 64
	x = int(x)
	return x

def cosFix16(x):
	x = math.cos(x * math.pi * 2 / table_size)
	x *= 64
	x = int(x)
	return x	

def twister_jump_func(i):
	i = int(i)
	y = -((i + sinFix16(i << 2) + 64 + ((cosFix16((i + 256) << 3) + 64) >> 1) ))
	return y

def twister_hjump_func(i):
	i = int(i)
	y = (sinFix16(i >> 1) - 64 + cosFix16((i + 256) << 3)) >> 4
	return y	

def dumpFunction(_precalc_func, display_name, f):
	f.write('const s16 ' + display_name + '[] =' + '\n')
	f.write('{' + '\n')

	cr = 0
	for angle in range(0,table_size):
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
	f.write('#define TWISTER_TABLE_SIZE ' + str(table_size) + '\n')
	f.write('\n')

	f.write('const s16 twister_jump_table[TWISTER_TABLE_SIZE];' + '\n')
	f.write('const s16 twister_hjump_table[TWISTER_TABLE_SIZE];' + '\n')

	f.close()

	##	Creates the C file
	f = codecs.open(filename_out + '.c', 'w')

	f.write('#include <genesis.h>\n\n')

	dumpFunction(_precalc_func = twister_jump_func, display_name = 'twister_jump_table', f = f)
	f.write('\n')

	dumpFunction(_precalc_func = twister_hjump_func, display_name = 'twister_hjump_table', f = f)
	f.write('\n')


	f.close()

main()