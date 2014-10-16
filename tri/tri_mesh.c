#include "tri_mesh.h"
#include <stdlib.h>

tri_mesh_t * tri_mesh_fromfile(FILE * f)
{
    tri_mesh_t * mesh = (tri_mesh_t *)malloc(sizeof(tri_mesh_t));
    tri_vec_t * vertex_ptr;
    
    int i;
    
    fread(&mesh->todo1, sizeof(uint32_t), 1, f);
    fread(&mesh->todo2, sizeof(uint32_t), 1, f);
    fread(&mesh->todo3, sizeof(uint32_t), 1, f);
    fread(&mesh->todo4, sizeof(uint32_t), 1, f);
    
    fread(&mesh->num_vertices, sizeof(uint32_t), 1, f);
    
    mesh->vertices = (tri_vec_t *)malloc(sizeof(tri_vec_t) * mesh->num_vertices);
    vertex_ptr = mesh->vertices;
    
    for (i = 0; i < mesh->num_vertices; i++) {
        fread(&(vertex_ptr->x), sizeof(int32_t), 1, f);
        fread(&(vertex_ptr->y), sizeof(int32_t), 1, f);
        fread(&(vertex_ptr->z), sizeof(int32_t), 1, f);
        vertex_ptr++;
    }
    
    return mesh;
}

int tri_mesh_free(tri_mesh_t * mesh) 
{
    free (mesh->vertices);
    free (mesh);
    return 0;
}
