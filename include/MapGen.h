#include <string.h>

#include "./Util.h"

#ifndef MAPGEN_H
#define MAPGEN_H

int generate_map(uint8_t map_hex_radius);
uint8_t assign_biome(TileState* state);
float fbm_noise(CubeCoord coord, float scale, int octaves);
#endif

