#include <stdlib.h>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "../include/Globals.h"
#include "../include/Hashmap.h"

// Common Config
int WINDOW_WIDTH = 1920;
int WINDOW_HEIGHT = 1080;
float HEX_RADIUS = 60.0f;
TileDefinition** g_arr_tile_definitions = NULL;
uint8_t g_arr_tile_definitions_size = 0;

RgbColor biome_colors[11] = {
    {153,255,255,255}, // [BIOME_FRESHWATER]
    {51,51,255,255},   // [BIOME_MARINE]
    {0,204,102,255},   // [BIOME_TROPICAL_GRASSLAND]
    {135,169,54,255},  // [BIOME_TEMPERATE_GRASSLAND]
    {65,129,0,255},   // [BIOME_TEMPERATE_RAINFOREST]
    {0,102,0,255},     // [BIOME_TROPICAL_RAINFOREST]
    {0,102,102,255},   // [BIOME_BOREAL_FOREST]
    {204,204,0,255},   // [BIOME_DESERT]
    {204,255,255,255}, // [BIOME_ARCTIC_TUNDRA]
    {169,214,228,255}, // [BIOME_ALPINE_TUNDRA]
    {128,128,128,255}  // [BIOME_MOUNTAIN]
};

// Display
SDL_Window* g_window = NULL;
SDL_Renderer* g_renderer = NULL;
TTF_Font* g_font_heading;
TTF_Font* g_font_regular;

// Game State
struct hashmap* g_game_map = NULL;
float camera_offset_x = 0.0f;
float camera_offset_y = 0.0f;


float g_mouse_pos_x = 0.0f;
float g_mouse_pos_y = 0.0f;

void cleanup_globals(){
    free(g_arr_tile_definitions);    
    hashmap_free(g_game_map);
}

RgbColor get_biome_color(uint8_t curr_biome){
    RgbColor biome_color = {0,0,0,0};
    return biome_colors[curr_biome];
}


char* get_biome_name(uint8_t b){
    switch(b){
        case BIOME_FRESHWATER: return "FRESHWATER";
        case BIOME_MARINE: return "MARINE";
        case BIOME_TROPICAL_GRASSLAND: return "TROPICAL_GRASSLAND";
        case BIOME_TEMPERATE_GRASSLAND: return "TEMPERATE_GRASSLAND";
        case BIOME_TEMPERATE_RAINFOREST: return "TEMPERATE_RAINFOREST";
        case BIOME_TROPICAL_RAINFOREST: return "TROPICAL_RAINFOREST";
        case BIOME_BOREAL_FOREST: return "BOREAL_FOREST";
        case BIOME_DESERT: return "DESERT";
        case BIOME_ARCTIC_TUNDRA: return "ARCTIC_TUNDRA";
        case BIOME_ALPINE_TUNDRA: return "ALPINE_TUNDRA";
        case BIOME_MOUNTAIN: return "MOUNTAIN";
        default: return "UNK";
    }
}