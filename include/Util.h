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
    NEED_DEFINITION
} ResourceFiles;

uint32_t determine_rand_val(int min, int max);

uint8_t determine_rand_percent();

int read_definition_from_res_file(ResourceFiles file, char*** target, uint8_t* target_size);

#endif