import os
import string
import codecs
import ast
import math

filename_out			=	"../../src/logo_yscroll_table"
table_size				=	64

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
	x = int(EaseInOutQuick(x / table_size) * 1024)
	x = (x >> 4) - 64
	return x

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
	f.write('#define LOGO_TRAJ_LEN ' + str(table_size) + '\n')
	f.write('\n')

	f.write('const s16 logo_yscroll_table[LOGO_TRAJ_LEN];' + '\n')

	f.close()

	##	Creates the C file
	f = codecs.open(filename_out + '.c', 'w')

	f.write('#include <genesis.h>\n\n')

	dumpFunction(_precalc_func = premul_fun, display_name = 'logo_yscroll_table', f = f)
	f.write('\n')

	f.close()

main()