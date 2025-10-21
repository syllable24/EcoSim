// globals.h
#ifndef GLOBALS_H
#define GLOBALS_H

#include <stdint.h>
#include <stdbool.h>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "hashmap.h"
#include "HexGridMath.h"
#include "Population.h"

#define TARGET_FPS 60
#define TARGET_FRAME_TIME_MS (1000 / TARGET_FPS)

#define FRAMES_PER_GAME_DAY (TARGET_FPS * 5)

#define HEX_GRID_RADIUS 20

#define MAX_NAME_LENGTH 1024

#define RIVER_MIN_LENGTH 6
#define RIVER_MAX_LENGTH 15
#define RIVER_MIN_AMOUNT 10
#define RIVER_MAX_AMOUNT 15

#define LARGE_LAKE_MIN_SIZE 2
#define LARGE_LAKE_MAX_SIZE 4

#define MOUNTAIN_CHAIN_MIN_AMOUNT 10
#define MOUNTAIN_CHAIN_MAX_AMOUNT 20
#define MOUNTAIN_CHAIN_MIN_LENGTH 5
#define MOUNTAIN_CHAIN_MAX_LENGTH 15

#define MARINE_CHAIN_MIN_AMOUNT 1
#define MARINE_CHAIN_MAX_AMOUNT 3
#define MARINE_CHAIN_MIN_LENGTH 4
#define MARINE_CHAIN_MAX_LENGTH 7

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
#define BIOME_MOUNTAIN 11

#define OWNER_NONE 0
#define OWNER_PLAYER 1

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
    float base_air_quality;    
    float base_soil_quality;
    float base_temperature;
    float base_moisture;
} TileDefinition;

typedef struct {
    CubeCoord coord;
    uint8_t tile_biome;
    TileDefinition* tile_def;    
    bool has_river;
    CubeCoord river_source;
    CubeCoord river_destination;
    PopulationUnit pop_unit;
	uint8_t owned_by;
} TileState;

// Display
extern SDL_Window* g_window;
extern SDL_Renderer* g_renderer;
extern TTF_Font* g_font_heading;
extern TTF_Font* g_font_regular;

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

extern uint16_t g_game_day_frame_counter;
extern uint8_t g_game_day;
extern uint8_t g_game_month;
extern uint16_t g_game_year;

extern CubeCoord** g_game_map_spiral_coords;

RgbColor get_biome_color(uint8_t curr_biome);
char* get_biome_name(uint8_t b);
void cleanup_globals();
char* get_biome_display_text(uint8_t b);

#endif