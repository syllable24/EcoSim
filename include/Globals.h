#include <stdint.h>
#include <stdbool.h>

#include "../include/hashmap.h"
#include "../include/HexGridMath.h"

// globals.h
#ifndef GLOBALS_H
#define GLOBALS_H

#define HEX_GRID_RADIUS 10
#define NUM_BIOME_SEEDS 8

// Common structs
typedef enum {
    FRESHWATER = 1,
    MARINE = 2,
    TROPICAL_GRASSLAND = 3,
    TEMPERATE_GRASSLAND = 4,
    TEMPERATE_RAINFOREST = 5,
    TROPICAL_RAINFOREST = 6,
    BOREAL_FOREST = 7,
    DESERT = 8,
    ARCTIC_TUNDRA = 9,
    ALPINE_TUNDRA = 10,
    COUNT = 11
} Biome;

typedef struct {
    char* name;
    char* texture;
    uint8_t base_water_quality;
    uint8_t base_light_quality;
    uint8_t base_air_quality;
    uint8_t base_soil_quality;
    uint8_t base_temperature_mod;
} TileDefinition;

typedef struct {
    const CubeCoord coord;
    const Biome tile_biome;
    const TileDefinition* tile_def;
    bool selected;    
} TileState;


// Common Config
extern int WINDOW_WIDTH;
extern int WINDOW_HEIGHT;
extern float HEX_RADIUS;
extern TileDefinition** g_arr_tile_definitions;
extern uint8_t g_arr_tile_definitions_size;


// Game State
extern struct hashmap* g_game_map;
extern float camera_offset_x;
extern float camera_offset_y;

extern float g_mouse_pos_x;
extern float g_mouse_pos_y;

char* get_biome_name(Biome b);
void cleanup_globals();

#endif