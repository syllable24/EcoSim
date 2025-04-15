#include <stdint.h>

#ifndef UTIL_H
#define UTIL_H

#define MAX_NAME_LENGTH 1024

// Custom log categories
#define LOG_CAT_UTIL SDL_LOG_CATEGORY_CUSTOM
#define LOG_CAT_DISPLAY SDL_LOG_CATEGORY_CUSTOM + 1
#define LOG_CAT_POPULATION SDL_LOG_CATEGORY_CUSTOM + 2
#define LOG_CAT_MAIN SDL_LOG_CATEGORY_CUSTOM + 3

typedef enum {
    NEED_DEFINITION,
    TILE_DEFINITION
} ResourceFiles;

typedef struct {
    char* name;
    char* texture;
    uint8_t base_water_quality;
    uint8_t base_light_quality;
    uint8_t base_air_quality;
    uint8_t base_soil_quality;
    uint8_t base_temperature_mod;
} TileDefinition;

uint32_t determine_rand_val(int min, int max);

uint8_t determine_rand_percent();

int read_definition_from_res_file(ResourceFiles file, char*** target, uint8_t* target_size);

int read_tile_definition(TileDefinition*** target, uint8_t* target_size);

void log_with_timestamp(void* userdata, int category, SDL_LogPriority priority, const char* message);

#endif
