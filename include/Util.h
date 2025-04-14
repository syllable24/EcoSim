#include <stdint.h>

#ifndef UTIL_H
#define UTIL_H

#define MAX_NAME_LENGTH 1024

typedef enum {
    NEED_DEFINITION
} ResourceFiles;

uint32_t determine_rand_val(int min, int max);

uint8_t determine_rand_percent();

int read_definition_from_res_file(ResourceFiles file, char*** target, uint8_t* target_size);

#endif