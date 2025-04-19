#include <stdint.h>
#include <stdbool.h>

#include "../include/hashmap.h"
#include "../include/HexGridMath.h"

// globals.h
#ifndef GLOBALS_H
#define GLOBALS_H

#define HEX_GRID_RADIUS 20

#define BIOME_FRESHWATER 1
#define BIOME_MARINE 2
#define BIOME_TROPICAL_GRASSLAND 3
#define BIOME_TEMPERATE_GRASSLAND 4
#define BIOME_TEMPERATE_RAINFOREST 5
#define BIOME_TROPICAL_RAINFOREST 6
#define BIOME_BOREAL_FOREST 7
#define BIOME_DESERT 8
#define BIOME_ARCTIC_TUNDRA 9
#define BIOME_ALPINE_TUNDRA 10

// Common structs
typedef struct {
    float r;
    float g;
    float b;
    float a;
} RgbColor;

extern RgbColor biome_colors[11];

typedef struct {
    char* name;
    char* texture;
    float base_water_quality;
    float base_light_quality;
    float base_air_quality;
    float base_soil_quality;
    float base_temperature;
    float base_moisture;
} TileDefinition;

typedef struct {
    CubeCoord coord;
    uint8_t tile_biome;
    TileDefinition* tile_def;
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

RgbColor get_biome_color(uint8_t curr_biome);
char* get_biome_name(uint8_t b);
void cleanup_globals();

#endif