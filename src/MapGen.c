#include <stdlib.h>
#include <stdio.h> 
#include <math.h> 
#include <SDL3/SDL.h>

#include "../include/HexGridMath.h"
#include "../include/MapGen.h"
#include "../include/Util.h"
#include "../include/Globals.h"

int game_map_hash_map_compare(const void *a, const void *b, void *udata){
    const TileState* ua = a;
    const TileState* ub = b;
    return (ua->coord.pos_q == ub->coord.pos_q 
            && ua->coord.pos_r == ub->coord.pos_r 
            && ua->coord.pos_s == ub->coord.pos_s) 
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

int generate_board(uint8_t map_hex_radius){
    SDL_LogTrace(LOG_CAT_MAPGEN, "Start generate_board()");
    int exit_status = SDL_APP_FAILURE;

    // Validate inputs
    if (!g_arr_tile_definitions || g_arr_tile_definitions_size <= 0 || map_hex_radius <= 0 ) {
        SDL_LogError(LOG_CAT_MAPGEN, "Invalid input: g_arr_tile_definitions=%p, g_arr_tile_definitions_size=%u, map_hex_radius=%u",
                     g_arr_tile_definitions, g_arr_tile_definitions_size, map_hex_radius);
        return SDL_APP_FAILURE;
    }        

    // Assumes symmetrical hexagonal flat-top hex-grid. (Big hexagon composed of smaller hexagons)
    uint64_t total_hex_count = hex_count_in_sprial(map_hex_radius);
    SDL_LogDebug(LOG_CAT_MAPGEN, "Calculating Hex Coords for %d hexes.", total_hex_count);

    g_game_map = hashmap_new(sizeof(TileState), total_hex_count, 0, 0, game_map_hash_map_hash, game_map_hash_map_compare, NULL, NULL);
    
    CubeCoord origin = {0,0,0};
    CubeCoord** spiral_coords = cube_sprial(origin, map_hex_radius);
    
    uint32_t rand_tile_def_id = determine_rand_val(0, g_arr_tile_definitions_size - 1);    

    SDL_LogDebug(LOG_CAT_MAPGEN, "Adding origin hex to g_game_map.");
    // Init center TileState
    hashmap_set(g_game_map, &(TileState){
        .coord = origin,
        .tile_def = g_arr_tile_definitions[rand_tile_def_id]
    });

    for (uint64_t curr_radius = 1; curr_radius <= map_hex_radius; curr_radius++){
        uint64_t hexes_in_ring = hex_count_in_ring(curr_radius);
        SDL_LogDebug(LOG_CAT_MAPGEN, "Adding %d hexes to g_game_map.", hexes_in_ring);

        for (uint64_t curr_ring_pos = 0; curr_ring_pos < hexes_in_ring; curr_ring_pos++){
            CubeCoord curr_coord = spiral_coords[curr_radius][curr_ring_pos];            

            rand_tile_def_id = determine_rand_val(0, g_arr_tile_definitions_size - 1);    
            // Init each TileState
            hashmap_set(g_game_map, &(TileState){
                .coord = curr_coord,
                .tile_def = g_arr_tile_definitions[rand_tile_def_id]
            });
        }        
    }

    exit_status = SDL_APP_CONTINUE;

cleanup:
    SDL_LogTrace(LOG_CAT_MAPGEN, "End generate_board()");
    return exit_status;
}