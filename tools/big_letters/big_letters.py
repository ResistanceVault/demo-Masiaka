import os
import string
import codecs
import ast
import math

filename_out			=	"../../src/logos_vertices"
table_size				=	512
table_size_pal 			=	(table_size * 60) // 50
fix16_precision			=	16
fixed_point_precision 	=	(1 << 16)

def createLogos():
	logos = [
				# TITAN
				[	'*** * ***  *  ** ',
					' *     *  * * * *',
					' *  *  *  *** * *',
					' *  *  *  * * * *',
					' *  *  *  * * * *'],

				# DRKLT
				[	'**  **  * * *  ***',
					'* * * * * * *   * ',
					'* * **  **  *   * ',
					'* * * * * * *   * ',
					'**  * * * * *** * '],

				# OXYRON
				[	' *  * * * * **   *  ** ',
					'* * * * * * * * * * * *',
					'* *  *  *** **  * * * *',
					'* * * *  *  * * * * * *',
					' *  * *  *  * *  *  * *'],										

				# TRBL
				[	'*** **  **  *  ',
					' *  * * * * *  ',
					' *  **  **  *  ',
					' *  * * * * *  ',
					' *  * * **  ***'],

				# DESIRE
				[	'**  **  ** * **  **',
					'* * *  *   * * * * ',
					'* * **  *  * **  **',
					'* * *    * * * * * ',
					'**  ** **  * * * **'],

				# LEMON
				[	'*   *** * *  *  ** ',
					'*   *   *** * * * *',
					'*   *** * * * * * *',
					'*   *   * * * * * *',
					'*** *** * *  *  * *'],

				# LIVE!
				[	'*   * * * *** *',
					'*     * * *   *',
					'*   * * * *** *',
					'*   * * * *    ',
					'*** *  *  *** *'],			

				# BLAST!
				[	'**  *    *   ** *** *',
					'* * *   * * *    *  *',
					'**  *   ***  *   *  *',
					'* * *   * *   *  *   ',
					'**  *** * * **   *  *'],

				# SIGFLUP
				[	' ** *  ** ** *  * * ** ',
					'*     *   *  *  * * * *',
					' *  * * * ** *  * * ** ',
					'  * * * * *  *  * * *  ',
					'**  *  *  *  **  *  *  '],

				# CONDENSE
				[	'**   *  **   ** * *',
					'* * * * * * *   * *',
					'**  * * **   *   * ',
					'*   * * *     *  * ',
					'*    *  *   **   * ']					
				]

	return logos

def Clamp(k, a, b):
	k = min(k, b)
	k = max(k, a)
	return k

def  main():

	maxwidth = max(len(line) for logo in createLogos() for line in logo)

	print("maxwidth = " + str(maxwidth))

	logo_vertice = []
	for logo in createLogos():
		new_vertice_list = []
		char_pos_y = 0
		logo_width = len(logo[0]) + 1
		for line in logo:
			char_pos_x = 0
			for c in line:
				char_pos_x += 1
				if c == '*':
					t_char_pos_x = ((320 - logo_width * 12) / 2) + (char_pos_x * 12) - 8
					t_char_pos_y = (char_pos_y * 10) + 100 - (char_pos_x * 2);
					t_char_pos_x += 0x80
					t_char_pos_y += 0x80
					t_char_pos_x = int(t_char_pos_x)
					t_char_pos_y = int(t_char_pos_y)
					new_vertice_list.append([t_char_pos_x, t_char_pos_y])
			char_pos_y += 1
		logo_vertice.append(new_vertice_list)

	print("found " + str(len(logo_vertice)) + " logo(s).")

	##	Creates the H file
	f = codecs.open(filename_out + '.h', 'w')

	f.write('#include <genesis.h>\n\n')

	f.write('#define MAX_VBALL_LOGO ' + str(len(createLogos())) + '\n\n')
	logo_idx = 0
	max_len = 0
	for logo in logo_vertice:
		f.write('#define LOGO_VLEN_' + str(logo_idx) + ' ' + str(len(logo_vertice[logo_idx])) + '\n')
		max_len = max(max_len, len(logo_vertice[logo_idx]))
		logo_idx += 1

	f.write('#define MAX_LOGO_VLEN ' + str(max_len) + '\n')

	f.write('\n')

	logo_idx = 0
	for logo in logo_vertice:
		f.write('const u16 logo_' + str(logo_idx) + '[LOGO_VLEN_' + str(logo_idx) + ' * 2];\n')	
		logo_idx += 1

	f.close()

	##	Creates the C file
	f = codecs.open(filename_out + '.c', 'w')

	f.write('#include <genesis.h>\n')
	f.write('#include "' + filename_out + '.h"' + '\n')
	f.write('\n')

	logo_idx = 0
	for logo in logo_vertice:
		# pretty comment
		f.write('/*\n')
		for line in createLogos()[logo_idx]:
			f.write('\t' + line + '\n')
		f.write('*/\n')
		f.write('\n')

		f.write('const u16 logo_' + str(logo_idx) + '[LOGO_VLEN_' + str(logo_idx) + ' * 2] = {\n')

		out_str = ''
		vertex_idx = 0
		last_y = 0
		for vertex in logo:
			if vertex[1] > last_y:
				out_str += '\n\t'
			out_str += str(vertex[0]) + ', ' + str(vertex[1])
			if vertex_idx < len(logo) - 1:
				out_str += ', '
			last_y = vertex[1]
			vertex_idx += 1
		f.write('\t' + out_str)

		f.write('\n};\n')

		f.write('\n')

		logo_idx += 1

	f.close()

main()