#include <string.h>

#include "./Util.h"

#ifndef MAPGEN_H
#define MAPGEN_H

int generate_board(uint8_t map_hex_radius);
void init_tile_states(uint8_t map_hex_radius);
uint8_t assign_biome(CubeCoord coord);
#endif

