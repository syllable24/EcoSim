#include <stdbool.h>
#include <SDL3/SDL.h>

#include "../include/Hashmap.h"

#ifndef DISPLAY_H
#define DISPLAY_H

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080
#define HEX_RADIUS 45.0f  // Radius (center to vertex)
#define HEX_COUNT_X 50
#define HEX_COUNT_Y 25

typedef struct {
    char* name;
    SDL_Texture* texture;
} TextureHashMapRecord;

int load_textures(SDL_Renderer* renderer, TileDefinition** tile_definitions, uint8_t tile_definition_size, struct hashmap** texture_map);

void get_hexagon_vertices(SDL_FPoint* points, float center_x, float center_y, float radius);

void draw_hexagon_outline(SDL_Renderer* renderer, SDL_FPoint* vertices);

int draw_hexagon_texture(SDL_Renderer* renderer, SDL_Texture* texture, SDL_FPoint points[6], float center_x, float center_y);

int draw_tile_map(SDL_Renderer* renderer, TileState** g_game_board, struct hashmap* texture_map, uint8_t arr_textures_size, uint8_t map_size_x, uint8_t map_size_y);

int texture_hash_map_compare(const void *a, const void *b, void *udata);

bool texture_hash_map_iter(const void *item, void *udata);

uint64_t texture_hash_map_hash(const void *item, uint64_t seed0, uint64_t seed1);

#endif