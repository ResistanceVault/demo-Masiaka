import os
import string
import codecs
import ast
import math
from vector3 import Vector3

folder_in = "vectorballs"
scale_factor = 10.0

def parse_obj_vector(_string, _scale_factor = 1.0):
	_args = _string.split(' ')
	_vector = Vector3(float(_args[1]), float(_args[2]), float(_args[3]))
	_vector *= _scale_factor
	_vector.x = float(_vector.x)
	_vector.y = float(_vector.y)
	_vector.z = float(_vector.z)
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
	filename_out = '../../src/ball_coords.h'
	fh = codecs.open(filename_out, 'w')

	filename_out = '../../src/ball_coords.c'
	fc = codecs.open(filename_out, 'w')
	fc.write('#include "genesis.h"\n\n')

	max_vertex_count = 0
	max_object_count = 0

	for filename_in in os.listdir(folder_in):
		if filename_in.find(".obj") > -1:
			face_list = []
			vertex_list = []
			vertex_normal_list = []
			edge_list = []
			bounding_distance = 0.0
			max_object_count += 1

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

			if len(vertex_list) > max_vertex_count:
				max_vertex_count = len(vertex_list)

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

			fh.write('#define ' + obj_name + '_VTX_COUNT ' + str(len(vertex_list)) + '\n')
			fh.write('const Vect3D_f16 vb_' + obj_name + '_vertex_pos[' + str(len(vertex_list)) + '];\n')


			##  Creates the C file that lists the meshes
			############################################
			fc.write('/* ' + filename_in + ' */' + '\n')
			fc.write('/* List of vertices */' + '\n')
			fc.write('const Vect3D_f16 vb_' + obj_name + '_vertex_pos[' + str(len(vertex_list)) + '] =\n')
			fc.write('{\n')

			##  Iterate on vertices
			for _vertex in vertex_list:
				_str_out = '{FIX16(' + str(_vertex.x) + '),\t' + 'FIX16(' + str(_vertex.z) + '),\t' + 'FIX16(' + str(_vertex.y * -1.0) + ')},'
				fc.write('\t' + _str_out + '\n')

			_str_out = '};'
			fc.write(_str_out + '\n')

	fh.write('\n')
	fh.write('#define MAX_VTX_COUNT ' + str(max_vertex_count) + '\n')
	fh.write('#define MAX_VBALL_OBJECTS ' + str(max_object_count) + '\n')

	fh.close()
	fc.close()
	f.close()


main()