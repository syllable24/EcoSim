#include <stdlib.h>
#include <stdio.h> 
#include <math.h> 
#include <SDL3/SDL.h>

#include "../include/MapGen.h"
#include "../include/Util.h"
#include "../include/Globals.h"

int game_map_hash_map_compare(const void *a, const void *b, void *udata){
    const TileState* ua = a;
    const TileState* ub = b;
    return (ua->axial_coord.pos_q == ub->axial_coord.pos_q && ua->axial_coord.pos_r == ub->axial_coord.pos_r) 
           ? 0  // equal 
           : 1; // different
}

bool game_map_hash_map_iter(const void *item, void *udata){
    const TileState* rec = item;    
    return true;
}

uint64_t game_map_hash_map_hash(const void *item, uint64_t seed0, uint64_t seed1) {
    const TileState* rec = item;
    // Use fixed seeds for deterministic hashing
    seed0 = 0x1234567890abcdefULL;
    seed1 = 0xfedcba0987654321ULL;
    return hashmap_sip((const AxialCoord*)rec, sizeof(AxialCoord), seed0, seed1);
}

int generate_board(uint8_t map_size_x, uint8_t map_size_y){
    SDL_LogTrace(LOG_CAT_MAPGEN, "Start generate_board()");
    int exit_status = SDL_APP_FAILURE;

    // Validate inputs
    if (!g_arr_tile_definitions || g_arr_tile_definitions_size <= 0 || map_size_x <= 0 || map_size_y <= 0) {
        SDL_LogError(LOG_CAT_MAPGEN, "Invalid input: game_board=%p, tile_defs=%p, tile_defs_size=%u, size=%ux%u",
                     g_game_board, g_arr_tile_definitions, g_arr_tile_definitions_size, map_size_x, map_size_y);
        return SDL_APP_FAILURE;
    }        

    // Assumes symmetrical hexagonal flat-top hex-grid. (Big hexagon composed of smaller hexagons)
    uint64_t total_hex_count = 1 + (3 * map_size_x * (map_size_x - 1));
    g_game_map = hashmap_new(sizeof(TileState), total_hex_count, 0, 0, game_map_hash_map_hash, game_map_hash_map_compare, NULL, NULL);

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

void clear_game_map(){
    hashmap_free(g_game_map);
}