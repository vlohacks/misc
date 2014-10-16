/* 
 * File:   tri_mesh.h
 * Author: vlo
 *
 * Created on 27. September 2014, 23:32
 */

#ifndef TRI_MESH_H
#define	TRI_MESH_H

#include "tri_3d.h"
#include <stdio.h>

typedef struct tri_mesh_s {
    uint32_t todo1, todo2, todo3, todo4;
    uint32_t num_vertices;
    tri_vec_t * vertices;
    struct tri_mesh_s ** sub_meshes;
} tri_mesh_t;

tri_mesh_t * tri_mesh_fromfile(FILE * f);
int tri_mesh_free(tri_mesh_t * mesh);

#endif	/* TRI_MESH_H */

