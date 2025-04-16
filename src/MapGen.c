#include <stdlib.h>
#include <stdio.h> 
#include <math.h> 
#include <SDL3/SDL.h>

#include "../include/MapGen.h"
#include "../include/Util.h"
#include "../include/Globals.h"

int generate_board(
    uint8_t map_size_x, 
    uint8_t map_size_y
){
    SDL_LogTrace(LOG_CAT_MAPGEN, "Start generate_board()");
    int exit_status = SDL_APP_FAILURE;

    // Validate inputs
    if (!g_arr_tile_definitions || g_arr_tile_definitions_size <= 0 || map_size_x <= 0 || map_size_y <= 0) {
        SDL_LogError(LOG_CAT_MAPGEN, "Invalid input: game_board=%p, tile_defs=%p, tile_defs_size=%u, size=%ux%u",
                     g_game_board, g_arr_tile_definitions, g_arr_tile_definitions_size, map_size_x, map_size_y);
        return SDL_APP_FAILURE;
    }        

    // Allocate rows
    g_game_board = malloc(map_size_x * sizeof(TileState*));    
    if(!g_game_board){
        SDL_LogError(LOG_CAT_MAPGEN, "Memory allocation for game board failed (%u entries, %zu bytes).", map_size_x, map_size_x * sizeof(TileState*));
        goto cleanup;
    }

    // Init rows
    for (uint8_t i = 0; i < map_size_x; i++){
        // Allocate each columns
        g_game_board[i] = malloc(map_size_y * sizeof(TileState));
        if(!g_game_board[i]){
            SDL_LogError(LOG_CAT_MAPGEN, "Memory allocation for game board failed (%u entries, %zu bytes).", map_size_y, map_size_y * sizeof(TileState));
            goto cleanup;
        }

        // Init each TileState
        for (uint8_t j = 0; j < map_size_y; j++){                        
            uint32_t tile_def_id = determine_rand_val(0, g_arr_tile_definitions_size - 1);
            g_game_board[i][j].tile_def = g_arr_tile_definitions[tile_def_id];
        }
    }

    exit_status = SDL_APP_CONTINUE;

cleanup:
    if (exit_status != SDL_APP_CONTINUE && g_game_board){
        for (uint8_t i = 0; i < map_size_x; i++){
            if((g_game_board)[i]){
                // Note: Do NOT free tile_def, as it points to g_arr_tile_definitions
                free(g_game_board[i]);
            }
        }
        free(g_game_board);
        g_game_board = NULL;
    }

    SDL_LogTrace(LOG_CAT_MAPGEN, "End generate_board()");
    return exit_status;
}

