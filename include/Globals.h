#include <stdint.h>
#include <stdbool.h>

#include "../include/hashmap.h"
#include "../include/HexGridMath.h"

// globals.h
#ifndef GLOBALS_H
#define GLOBALS_H

#define HEX_GRID_RADIUS 10

// Common structs
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

void cleanup_globals();

#endif