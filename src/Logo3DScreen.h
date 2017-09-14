#include "genesis.h"

#define RSE_LOGO_3D_MAX_POINTS  256

#define RSE_COPY_VECTOR2D(D, S) D.x = S.x; D.y = S.y  

#define RSE_LOGO_3D_LOAD_MESH(mesh_name) mesh_coord = mesh_name ## _coord; mesh_poly_ind = mesh_name ## _poly_ind; mesh_face_norm = mesh_name ## _face_norm; vtx_count = mesh_name ## _VTX_COUNT; poly_count = mesh_name ## _FACE_COUNT;
