#include <string.h>

#include "./Util.h"

#ifndef MAPGEN_H
#define MAPGEN_H

int generate_board(TileState*** arr_game_board, TileDefinition** arr_tile_definitions, uint8_t arr_tile_definition_size, uint8_t map_size_x, uint8_t map_size_y);

#endif

