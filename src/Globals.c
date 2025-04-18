#include <stdlib.h>

#include "../include/Globals.h"
#include "../include/Hashmap.h"

// Common Config
int WINDOW_WIDTH = 1920;
int WINDOW_HEIGHT = 1080;
float HEX_RADIUS = 60.0f;
TileDefinition** g_arr_tile_definitions = NULL;
uint8_t g_arr_tile_definitions_size = 0;

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

char* get_biome_name(Biome b){
    switch(b){
        case FRESHWATER: return "FRESHWATER";
        case MARINE: return "MARINE";
        case TROPICAL_GRASSLAND: return "TROPICAL_GRASSLAND";
        case TEMPERATE_GRASSLAND: return "TEMPERATE_GRASSLAND";
        case TEMPERATE_RAINFOREST: return "TEMPERATE_RAINFOREST";
        case TROPICAL_RAINFOREST: return "TROPICAL_RAINFOREST";
        case BOREAL_FOREST: return "BOREAL_FOREST";
        case DESERT: return "DESERT";
        case ARCTIC_TUNDRA: return "ARCTIC_TUNDRA";
        case ALPINE_TUNDRA: return "ALPINE_TUNDRA";
        default: return "UNK";
    }
}