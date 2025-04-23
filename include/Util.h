#include <stdint.h>

#include "../include/Globals.h"
#include "../include/HexGridMath.h"

#ifndef UTIL_H
#define UTIL_H

#define MAX_NAME_LENGTH 1024

// Custom log categories
#define LOG_CAT_UTIL SDL_LOG_CATEGORY_CUSTOM
#define LOG_CAT_DISPLAY SDL_LOG_CATEGORY_CUSTOM + 1
#define LOG_CAT_POPULATION SDL_LOG_CATEGORY_CUSTOM + 2
#define LOG_CAT_MAIN SDL_LOG_CATEGORY_CUSTOM + 3
#define LOG_CAT_MAPGEN SDL_LOG_CATEGORY_CUSTOM + 4
#define LOG_CAT_HEXMATH SDL_LOG_CATEGORY_CUSTOM + 5

typedef enum {
    NEED_DEFINITION,
    TILE_DEFINITION
} ResourceFiles;

int32_t determine_rand_val(int min, int max);

int8_t determine_rand_percent();

int read_definition_from_res_file(ResourceFiles file, char*** target, uint8_t* target_size);

int read_tile_definition(TileDefinition*** target, uint8_t* target_size);

void log_with_timestamp(void* userdata, int category, SDL_LogPriority priority, const char* message);

char* print_coord(CubeCoord coord);

void log_tile_state_string(const TileState* state);

void log_tile_state_string_to_file(const TileState* state, char* file_name);

bool is_same_coord(const CubeCoord* a, const CubeCoord* b);

void shuffle_array(void* base, size_t n_items, size_t size) ;

#endif
