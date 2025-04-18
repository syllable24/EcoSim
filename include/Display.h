#include <stdbool.h>
#include <SDL3/SDL.h>

#include "../include/Globals.h"
#include "../include/Hashmap.h"

#ifndef DISPLAY_H
#define DISPLAY_H

typedef struct {
    char* name;
    SDL_Texture* texture;
} TextureHashMapRecord;

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;    
} RgbColor;

int load_textures(SDL_Renderer* renderer, TileDefinition** tile_definitions, uint8_t tile_definition_size);

void get_hexagon_vertices(SDL_FPoint* points, float center_x, float center_y, float radius);

void draw_hexagon_outline(SDL_Renderer* renderer, SDL_FPoint* vertices);

int draw_hexagon_texture(SDL_Renderer* renderer, SDL_Texture* texture, SDL_FPoint points[6], float center_x, float center_y);

int draw_hexagon_solid_color(SDL_Renderer* renderer, RgbColor color, SDL_FPoint points[6], float center_x, float center_y);

int draw_tile_map(SDL_Renderer* renderer, uint8_t map_hex_radius);

int frame_update(SDL_Renderer* renderer);

void handle_left_click(SDL_Renderer* renderer);

int texture_hash_map_compare(const void *a, const void *b, void *udata);

bool texture_hash_map_iter(const void *item, void *udata);

uint64_t texture_hash_map_hash(const void *item, uint64_t seed0, uint64_t seed1);

void clear_display_state();

#endif