import os
import string
import codecs
import ast
import math

filename_out			=	"../../src/sine_scroll"
table_size				=	1024

def premul_fun(x):
	def sinFix16(i):
		return int(math.sin(i * 2 * math.pi / table_size) * 64)
	x = int(x)
	y = (sinFix16(x << 3) >> 3) + (sinFix16((x + 123) << 5) >> 5)
	return int(y)

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
	f.write('#define SINE_SCROLL_LEN ' + str(table_size) + '\n')
	f.write('\n')

	f.write('const s16 sine_scroll[SINE_SCROLL_LEN];' + '\n')

	f.close()

	##	Creates the C file
	f = codecs.open(filename_out + '.c', 'w')

	f.write('#include <genesis.h>\n\n')

	dumpFunction(_precalc_func = premul_fun, display_name = 'sine_scroll', f = f)
	f.write('\n')

	f.close()

main()