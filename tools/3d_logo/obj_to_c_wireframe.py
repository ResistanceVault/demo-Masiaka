import os
import string
import codecs
import ast
import math
from vector3 import Vector3

folder_in = "meshes"
scale_factor = 10.0
scale_vector = Vector3(1,1.175,1.0)
position_vector = Vector3(0,-0.1,0)

def parse_obj_vector(_string, _scale_factor = 1.0):
	_args = _string.split(' ')
	_vector = Vector3(float(_args[1]), float(_args[2]), float(_args[3]))
	_vector.x = float(_vector.x)
	_vector.y = float(_vector.y)
	_vector.z = float(_vector.z)
	_vector *= _scale_factor
	return _vector

def parse_obj_face(_string):
	## f 13//1 15//2 4//3 2//4
	_args = _string.split(' ')
	_args.pop(0)
	_face = []
	_vertex_index = -1
	_uv_index = -1
	_normal_index = -1
	for _arg in _args:
		_corner = _arg.split('/')
		_vertex_index = -1
		_uv_index = -1
		_normal_index = -1
		if len(_corner) > 0:
			if _corner[0] != '':
				_vertex_index = int(_corner[0]) - 1
			if _corner[1] != '':
				_uv_index = int(_corner[1]) - 1
			if _corner[2] != '':
				_normal_index = int(_corner[2]) - 1

		_face.append({'vertex':_vertex_index, 'uv':_uv_index, 'normal':_normal_index})

	# _face = _face[::-1]
	# _face.append(_face.pop(0))
	# _face.append(_face[0])

	# if len(_face) == 3:
	# 	_face.append(_face[:-1])

	return _face

def main():
	filename_out = '../../src/logo_meshs.h'
	fh = codecs.open(filename_out, 'w')

	filename_out = '../../src/logo_meshs.c'
	fc = codecs.open(filename_out, 'w')
	fc.write('#include "genesis.h"\n\n')
	fc.write('__attribute__((aligned(4)))\n\n')

	for filename_in in os.listdir(folder_in):
		if filename_in.find(".obj") > -1:
			face_list = []
			vertex_list = []
			vertex_normal_list = []
			edge_list = []
			bounding_distance = 0.0

			f = codecs.open(os.path.join(folder_in, filename_in), 'r')
			for line in f:
				# print(repr(line))
				if len(line) > 0:
					line = line.replace('\t', ' ')
					line = line.replace('  ', ' ')
					line = line.replace('  ', ' ')
					line = line.strip()
					if line.startswith('v '):
						# print('found a vertex')
						new_vertex = parse_obj_vector(line)
						dist_to_origin = math.sqrt(new_vertex.x * new_vertex.x + new_vertex.y * new_vertex.y + new_vertex.z * new_vertex.z)
						if dist_to_origin > bounding_distance:
							bounding_distance = dist_to_origin
						vertex_list.append(new_vertex)

					if line.startswith('vn '):
						# print('found a vertex normal')
						vertex_normal_list.append(parse_obj_vector(line))

					if line.startswith('f '):
						# print('found a face')
						face_list.append(parse_obj_face(line))

			new_edge = []
			for _face in face_list:
				for _corner in _face:
					new_edge.append(_corner['vertex'])
					if len(new_edge) == 2:
						if new_edge[0] > new_edge[1]:
							tmp = new_edge[1]
							new_edge[1] = new_edge[0]
							new_edge[0] = tmp
						already_in_list = False
						for _edge in edge_list:
							if _edge == new_edge:
								already_in_list = True
						if not already_in_list:
							edge_list.append(new_edge)
						new_edge = []

			if bounding_distance > 0.0:
				for i in range(len(vertex_list)):
					vertex_list[i] = vertex_list[i] * (scale_factor / bounding_distance)

			print("bounding_distance = " + str(bounding_distance))


			print('OBJ Parser : "' + filename_in + '", ' + str(len(vertex_list)) + ' vertices, ' + str(len(vertex_normal_list)) + ' normals, ' + str(len(face_list)) + ' faces, ')

			obj_name = filename_in.replace('.obj', '')
			obj_name = obj_name.replace(' ', '')
			obj_name = obj_name.replace('-', '_')
			obj_name = obj_name.lower()

			##  Creates the H file that lists the arrays
			############################################

			fh.write('#define ' + obj_name + '_VTX_COUNT ' + str(len(vertex_list) * 3) + '\n')
			fh.write('#define ' + obj_name + '_FACE_COUNT ' + str(len(face_list)) + '\n')
			fh.write('const Vect3D_f16 ' + obj_name + '_coord[' + str(len(vertex_list) * 3) + '];\n')
			fh.write('const short  ' + obj_name + '_poly_ind[' + str(len(face_list) * 4) + '];\n')
			fh.write('const u16  ' + obj_name + '_line_ind[' + str(len(edge_list) * 2) + '];\n')
			fh.write('const Vect3D_f16  ' + obj_name + '_face_norm[' + str(len(face_list)) + '];\n\n')


			##  Creates the C file that lists the meshes
			############################################
			fc.write('/* ' + filename_in + ' */' + '\n')
			fc.write('/* List of vertices */' + '\n')
			fc.write('const Vect3D_f16 ' + obj_name + '_coord[' + str(len(vertex_list) * 3) + '] =\n')
			fc.write('{\n')

			##  Iterate on vertices
			for _vertex in vertex_list:
				_x = _vertex.x + (position_vector.x * scale_factor)
				_y = _vertex.y + (position_vector.y * scale_factor)
				_z = _vertex.z + (position_vector.z * scale_factor)

				_x *= scale_vector.x
				_y *= scale_vector.y
				_z *= scale_vector.z

				_str_out = '{FIX16(' + str(_x) + '),\t' + 'FIX16(' + str(_z) + '),\t' + 'FIX16(' + str(_y * -1.0) + ')},'
				fc.write('\t' + _str_out + '\n')

			_str_out = '};'
			fc.write(_str_out + '\n')

			##  Iterate on faces
			fc.write('\n')
			fc.write('/* List of faces */' + '\n')

			fc.write('const short  ' + obj_name + '_poly_ind[' + str(len(face_list) * 4) + '] =\n')
			fc.write('{\n')

			for _face in face_list:
				_str_out = '\t'

				corner_idx = 0
				for _corners in _face:
					_str_out += str(_corners['vertex'])
					corner_idx += 1
					_str_out += ','

				if len(_face) == 3:
					_str_out += '-1,'

				fc.write(_str_out + '\n')

			_str_out = '};'
			fc.write(_str_out + '\n')
			fc.write('\n')

			##  Iterate on edges
			fc.write('/* List of edges */' + '\n')

			fc.write('const u16  ' + obj_name + '_line_ind[' + str(len(edge_list) * 2) + '] =\n')
			fc.write('{\n')

			for _edge in edge_list:
				_str_out = '\t'
				_str_out += str(_edge[0]) + ', ' + str(_edge[1]) + ', '
				fc.write(_str_out + '\n')

			_str_out = '};'
			fc.write(_str_out + '\n')
			fc.write('\n')

			##  Iterate on face normals
			fc.write('/* List of face normals */' + '\n')

			fc.write('const Vect3D_f16  ' + obj_name + '_face_norm[' + str(len(face_list)) + '] =\n')
			fc.write('{\n')

			for _face in face_list:
				_str_out = '\t'
				_vertex = vertex_normal_list[_face[0]['normal']]
				_str_out = '{FIX16(' + str(_vertex.x) + '),\t' + 'FIX16(' + str(_vertex.z) + '),\t' + 'FIX16(' + str(_vertex.y * -1.0) + ')},'
				fc.write('\t' + _str_out + '\n')

			_str_out = '};'
			fc.write(_str_out + '\n')
			fc.write('\n')


	fh.close()
	fc.close()
	f.close()


main()