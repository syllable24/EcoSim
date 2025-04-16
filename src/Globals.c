#include <stdlib.h>

#include "../include/Globals.h"

// Common Config
int WINDOW_WIDTH = 1920;
int WINDOW_HEIGHT = 1080;
float HEX_RADIUS = 45.0f;
TileDefinition** g_arr_tile_definitions = NULL;
uint8_t g_arr_tile_definitions_size = 0;

// Game State
TileState** g_game_board = NULL;
float camera_offset_x = 0.0f;
float camera_offset_y = 0.0f;

void cleanup_globals(){
    free(g_arr_tile_definitions);
    free(g_game_board);
}