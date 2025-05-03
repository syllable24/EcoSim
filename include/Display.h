#include <stdbool.h>
#include <SDL3/SDL.h>

#include "Globals.h"
#include "Hashmap.h"

#ifndef DISPLAY_H
#define DISPLAY_H

typedef struct {
    char* name;
    SDL_Texture* texture;
} TextureHashMapRecord;

int load_textures(TileDefinition** tile_definitions, uint8_t tile_definition_size);

void get_hexagon_vertices(SDL_FPoint* points, float center_x, float center_y, float radius);

void draw_hexagon_outline(SDL_FPoint* vertices);

int draw_hexagon_texture(SDL_Texture* texture, SDL_FPoint points[6], float center_x, float center_y);

int draw_tile_map(uint8_t map_hex_radius, SDL_FRect border);

int draw_menu(SDL_FRect border);

int draw_debug_info();

int frame_update(float delta_time);

void handle_left_click();

int texture_hash_map_compare(const void *a, const void *b, void *udata);

bool texture_hash_map_iter(const void *item, void *udata);

uint64_t texture_hash_map_hash(const void *item, uint64_t seed0, uint64_t seed1);

void clear_display_state();

int init_and_add_texture(char* texture_id, char* texture_filename);

int draw_hexagon(const TileState* curr_state, char* hex_def_name);

int draw_selected_tile_state_menu(const TileState* state);

int draw_text(SDL_Color color, TTF_Font* font, char* text, float x, float y);

#endif