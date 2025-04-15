#include <stdlib.h> 
#include <time.h> 
#include <stdio.h> 
#include <math.h> 
#include <SDL3/SDL.h>

#include "../include/Util.h"

// Calculate the six vertices of a flat-top hexagon
void get_hexagon_vertices(SDL_FPoint* points, float center_x, float center_y, float radius) {
    for (int i = 0; i < 6; i++) {
        float angle = (float)(M_PI / 3.0 * i);
        points[i].x = center_x + radius * cosf(angle);
        points[i].y = center_y + radius * sinf(angle);
    }
}

// Draw a hexagon outline by connecting vertices
void draw_hexagon_outline(SDL_Renderer* renderer, SDL_FPoint* vertices) {
    for (int i = 0; i < 6; i++) {
        int next = (i + 1) % 6;
        SDL_RenderLine(
            renderer, 
            vertices[i].x, vertices[i].y,
            vertices[next].x, vertices[next].y
        );
    }
}

int draw_hexagon_texture(SDL_Renderer* renderer, SDL_Texture* texture, SDL_FPoint points[6], float center_x, float center_y){
    // Define hexagon vertices and texture coordinates
    SDL_Vertex vertices[7];     

    // Center vertex
    vertices[0].position.x = center_x;
    vertices[0].position.y = center_y;
    vertices[0].tex_coord.x = 0.5f; // Center of texture
    vertices[0].tex_coord.y = 0.5f;
    vertices[0].color.r = 1.0f;
    vertices[0].color.g = 1.0f;
    vertices[0].color.b = 1.0f;
    vertices[0].color.a = 1.0f;

    // Outer vertices
    for (int i = 0; i < 6; i++) {
        vertices[i + 1].position = points[i];
        // Map texture coordinates to fit hexagon
        float tex_angle = (float)(M_PI / 3.0 * i);
        vertices[i + 1].tex_coord.x = 0.5f + 0.4f * cosf(tex_angle);
        vertices[i + 1].tex_coord.y = 0.5f + 0.4f * sinf(tex_angle);
        vertices[i + 1].color.r = 1.0f;
        vertices[i + 1].color.g = 1.0f;
        vertices[i + 1].color.b = 1.0f;
        vertices[i + 1].color.a = 1.0f;
    }

    // Define triangle indices for fan
    int indices[] = { 0, 1, 2, 0, 2, 3, 0, 3, 4, 0, 4, 5, 0, 5, 6, 0, 6, 1 };

    if (SDL_RenderGeometry(renderer, texture, vertices, 7, indices, 18) < 0) {
        SDL_LogError(LOG_CAT_DISPLAY, "RenderGeometry failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    return SDL_APP_CONTINUE;
}

/*
int load_textures(SDL_Texture** textures){
    SDL_LogTrace(LOG_CAT_DISPLAY, "Start load_textures()");
    
    SDL_Surface* grass_bmp = SDL_LoadBMP("img/green-grass-texture.bmp");
    if (grass_bmp == NULL){
        SDL_LogError(LOG_CAT_DISPLAY, "Could not load green-grass-texture.bmp.");
        return SDL_APP_FAILURE;
    }
    
    grass_texture = SDL_CreateTextureFromSurface(renderer, grass_bmp);
    SDL_DestroySurface(grass_bmp);
    if (grass_texture == NULL){
        SDL_LogError(LOG_CAT_DISPLAY, "Could not load texture green-grass-texture.png.");
        return SDL_APP_FAILURE;
    }
    
    SDL_Surface* ice_bmp = SDL_LoadBMP("img/Ice.bmp");
    if (ice_bmp == NULL){
        SDL_LogError(LOG_CAT_DISPLAY, "Could not load ice.bmp.");
        return SDL_APP_FAILURE;
    }
    
    ice_texture = SDL_CreateTextureFromSurface(renderer, ice_bmp);
    SDL_DestroySurface(ice_bmp);
    if (ice_texture == NULL){
        SDL_LogError(LOG_CAT_DISPLAY, "Could not load texture ice.bmp.");
        return SDL_APP_FAILURE;
    }

    SDL_Surface* stone_bmp = SDL_LoadBMP("img/Stone.bmp");
    if (stone_bmp == NULL){
        SDL_LogError(LOG_CAT_DISPLAY, "Could not load stone.bmp.");
        return SDL_APP_FAILURE;
    }
    
    stone_texture = SDL_CreateTextureFromSurface(renderer, stone_bmp);
    SDL_DestroySurface(stone_bmp);
    if (stone_texture == NULL){
        SDL_LogError(LOG_CAT_DISPLAY, "Could not load texture ice.bmp.");
        return SDL_APP_FAILURE;
    }

    SDL_Surface* desert_bmp = SDL_LoadBMP("img/Desert.bmp");
    if (desert_bmp == NULL){
        SDL_LogError(LOG_CAT_DISPLAY, "Could not load desert.bmp.");
        return SDL_APP_FAILURE;
    }
    
    desert_texture = SDL_CreateTextureFromSurface(renderer, desert_bmp);
    SDL_DestroySurface(desert_bmp);
    if (desert_texture == NULL){
        SDL_LogError(LOG_CAT_DISPLAY, "Could not load texture ice.bmp.");
        return SDL_APP_FAILURE;
    }

    SDL_LogTrace(LOG_CAT_DISPLAY, "End load_textures()");
    return SDL_APP_CONTINUE;
}
*/