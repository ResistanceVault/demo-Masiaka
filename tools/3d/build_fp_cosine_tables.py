import os
import string
import codecs
import ast
import math
from vector3 import Vector3

filename_out			=	"../../src/fp_cosine_table"
table_size				=	1024

def dumpCosine(_cosine_func, display_name, f):
	f.write('const fix16 ' + display_name + '[] =' + '\n')
	f.write('{' + '\n')

	cr = 0
	for angle in range(0,table_size):
		_cos = _cosine_func(angle * math.pi / (table_size / 2.0))
		_str_out = 'FIX16(' + str(_cos) + '),'
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
	f.write('#define FP_COSINE_TABLE_LEN ' + str(table_size) + '\n')
	f.write('\n')

	f.write('const fix16 fp_cos[FP_COSINE_TABLE_LEN];' + '\n')
	f.write('const fix16 fp_sin[FP_COSINE_TABLE_LEN];' + '\n')

	f.close()

	##	Creates the C file
	f = codecs.open(filename_out + '.c', 'w')

	f.write('#include <genesis.h>\n\n')

	dumpCosine(_cosine_func = math.cos, display_name = 'fp_cos', f = f)
	f.write('\n')
	dumpCosine(_cosine_func = math.sin, display_name = 'fp_sin', f = f)

	f.close()

main()