#include <string.h>

#include "./Util.h"

#ifndef MAPGEN_H
#define MAPGEN_H

int generate_map(uint8_t map_hex_radius);

uint8_t assign_biome(TileState* state);

float fbm_noise(CubeCoord coord, float scale, int octaves);

void generate_base_tiles(uint8_t map_hex_radius);

void generate_mountain_chain(CubeCoord* arr_mountain_coords, uint16_t* arr_mountain_coords_size);

void generate_marine_chain();

void generate_river(CubeCoord* arr_mountain_coords, uint16_t arr_mountain_coords_size);

const TileState* pick_seed_tile(CubeCoord* seed_coord, uint8_t filter);

typedef bool (*NeighborPredicate)(const TileState* tile, void* context);

bool any_neighbor_matches(CubeCoord center, NeighborPredicate predicate, void* context);

bool is_out_of_bounds(CubeCoord coord);

bool is_marine(const TileState* tile, void* context);

bool is_mountain(const TileState* tile, void* context);

bool is_marine_or_mountain(const TileState* tile, void* context);

bool is_same_coord(CubeCoord* a, CubeCoord* b);

#endif

