#include <stdio.h>

#include "podfile.h"
#include "tri_mesh.h"
#include <SDL2/SDL.h>

#define WSIZE_X 1024
#define WSIZE_Y 768

#define SSIZE_X 640
#define SSIZE_Y 480

typedef struct {
    SDL_Window * win;
    SDL_Renderer * ren;

    SDL_Texture * tex;
    SDL_Texture * tex2;
    SDL_Surface * sur;
    SDL_Surface * sur2;
} sdl_zeug_t;



int main(int argc, char ** argv) 
{
    int i, x, y;
    unsigned char r, g, b;
    int fu = 0;
    podfile_t * pod;
    pod = podfile_open(argv[1]);
    
    podfile_asset_t * asset_tex;
    podfile_asset_t * asset_pal;
    podfile_asset_t * asset_mesh;
    
    tri_mesh_t * mesh;
    tri_vec_t * vec_ptr;
    
    char tmp[32];
    char tmp_palette[768];
    char * data;
    
    sdl_zeug_t sdl_zeug;
    
    SDL_Event e;
    
    for (i = 0; i < pod->num_assets; i++) {
        if (pod->assets[i]->palette)
            printf("%-32s %-32s %8i %8i\n", pod->assets[i]->name, pod->assets[i]->palette, pod->assets[i]->size, pod->assets[i]->offset);
        else
            printf("%-32s %-32s %8i %8i\n", pod->assets[i]->name, "", pod->assets[i]->size, pod->assets[i]->offset);
    }
    
    
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER) != 0) {
            fprintf(stderr, "ERROR SDL_Init: %s\n", SDL_GetError());
            return 1;
    }

    sdl_zeug.win = SDL_CreateWindow("Yayy", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WSIZE_X, WSIZE_Y, SDL_WINDOW_SHOWN);
    if (sdl_zeug.win == 0) {
            fprintf(stderr, "ERROR SDL_CreateWindow: %s\n", SDL_GetError());
            SDL_Quit();
            return 1;
    }

    sdl_zeug.ren = SDL_CreateRenderer(sdl_zeug.win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (sdl_zeug.ren == 0) {
            fprintf(stderr, "ERROR SDL_CreateRenderer: %s\n", SDL_GetError());
            SDL_DestroyWindow(sdl_zeug.win);
            SDL_Quit();
            return 1;

    }
    
    sdl_zeug.tex = SDL_CreateTexture(sdl_zeug.ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SSIZE_X, SSIZE_Y);
    sdl_zeug.tex2 = SDL_CreateTexture(sdl_zeug.ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SSIZE_X, SSIZE_Y);
    sdl_zeug.sur = SDL_CreateRGBSurfaceFrom(0, SSIZE_X, SSIZE_Y, 32, 0, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
    sdl_zeug.sur2 = SDL_CreateRGBSurfaceFrom(0, SSIZE_X, SSIZE_Y, 32, 0, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
    
    //asset_tex = podfile_get_asset_by_name("ART\\CHIP1A.RAW", pod);
    *tmp = 0;
    strcat(tmp, "ART\\");
    strcat(tmp, asset_tex->palette);
    asset_pal = podfile_get_asset_by_name(tmp, pod);
    if (asset_pal) {
        podfile_get_asset_data(tmp_palette, pod, asset_pal);
    } else {
        fprintf(stderr, "asset %s not found!!\n", tmp);
    }
    
    data = (char *)malloc(asset_tex->size);
    podfile_get_asset_data(data, pod, asset_tex);
    
    printf("%s %s\n", asset_tex->name, asset_pal->name);
    
    asset_mesh = podfile_get_asset_by_name("MODELS\\CUBE.BIN", pod);
    
    mesh = tri_mesh_fromfile(podfile_get_asset_fileptr(pod, asset_mesh));
    
    printf("num_vert: %i\n", mesh->num_vertices);
    
    vec_ptr = mesh->vertices;
    for (i = 0; i < mesh->num_vertices; i++) {
        printf("%3i: x=%8i y=%8i z=%8i\n", i, vec_ptr->x, vec_ptr->y, vec_ptr->z);
        vec_ptr++;        
    }
    
    
    SDL_LockTexture(sdl_zeug.tex2, 0, &sdl_zeug.sur2->pixels, &sdl_zeug.sur2->pitch);
    SDL_LockTexture(sdl_zeug.tex, 0, &sdl_zeug.sur->pixels, &sdl_zeug.sur->pitch);    
    
    
    
    for (y = 0; y < 256; y++) {
        for (x = 0; x < 256; x++) {
            int i = data[y * 256 + x];
            r = tmp_palette[i * 3];
            g = tmp_palette[i * 3 + 1];
            b = tmp_palette[i * 3 + 2];
            
            ((uint32_t *)sdl_zeug.sur2->pixels)[y * SSIZE_X + x] = (0xff000000 | (r<<16) | (g<<8) | (b));
        }
    }
    
    free(data);
    asset_tex = podfile_get_asset_by_name("DATA\\ARTIC.RAW", pod);
    
    data = malloc(asset_tex->size);
    podfile_get_asset_data(data, pod, asset_tex);
    
    for (y = 0; y < 256; y++) {
        for (x = 0; x < 256; x++) {
            r = data[y * 256 + x];
            
//            ((uint32_t *)sdl_zeug.sur2->pixels)[y * SSIZE_X + x] = (0xff000000 | (r<<16) | (r<<8) | (r));
        }
    }
    
    
    SDL_BlitSurface(sdl_zeug.sur2, 0, sdl_zeug.sur, 0);
    SDL_UnlockTexture(sdl_zeug.tex2);
    SDL_UnlockTexture(sdl_zeug.tex);
    SDL_RenderCopy(sdl_zeug.ren, sdl_zeug.tex, 0, 0);
    SDL_RenderPresent(sdl_zeug.ren);
    
    while (SDL_WaitEvent(&e) && (!fu)) { //User requests quit 
            switch (e.type) {

            case SDL_QUIT:
                    printf("quit request\n");
                    fu = 1;
                    break;

            case SDL_USEREVENT:
                    //update_scene(&c);
                    break;

            default: 
                    break;

            }

            if (fu)
                break;

    }    

    SDL_Quit();    
    
    return 0;
}