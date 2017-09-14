import os
import string
import codecs
import ast
import math

filename_out			=	"../../src/div_premul"
table_size				=	4096

def premul_fun(x):
	if x == 0:
		return 0
	return 65535//x;

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

	f.write('#include <genesis.h>\n\n')
	f.write('#define DIV_PREMUL_LEN ' + str(table_size) + '\n')
	f.write('\n')

	f.write('const s16 div_table[DIV_PREMUL_LEN];' + '\n')

	f.close()

	##	Creates the C file
	f = codecs.open(filename_out + '.c', 'w')

	f.write('#include <genesis.h>\n\n')

	dumpFunction(_precalc_func = premul_fun, display_name = 'div_table', f = f)
	f.write('\n')

	f.close()

main()