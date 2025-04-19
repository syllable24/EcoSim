#include <stdlib.h>
#include <stdio.h> 
#include <math.h> 
#include <SDL3/SDL.h>

#include "../include/HexGridMath.h"
#include "../include/MapGen.h"
#include "../include/Util.h"
#include "../include/Globals.h"

#define STB_PERLIN_IMPLEMENTATION
#include "stb_perlin.h"

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
 
    // Init all TileStates
    init_tile_states(map_hex_radius);
    
    // Assign random Tile Definitons based on biome

    exit_status = SDL_APP_CONTINUE;

cleanup:
    SDL_LogTrace(LOG_CAT_MAPGEN, "End generate_board()");
    return exit_status;
}

void init_tile_states(uint8_t map_hex_radius){
    // Assumes symmetrical hexagonal flat-top hex-grid. (Big hexagon composed of smaller hexagons)
    uint64_t total_hex_count = hex_count_in_sprial(map_hex_radius);
    SDL_LogDebug(LOG_CAT_MAPGEN, "Calculating Hex Coords for %d hexes.", total_hex_count);

    g_game_map = hashmap_new(sizeof(TileState), total_hex_count, 0, 0, game_map_hash_map_hash, game_map_hash_map_compare, NULL, NULL);
    SDL_LogDebug(LOG_CAT_MAPGEN, "Hashmap init done.");
    
    CubeCoord origin_coord = {0,0,0};
    CubeCoord** spiral_coords = cube_sprial(origin_coord, map_hex_radius);

    uint32_t rand_tile_def_id = determine_rand_val(0, g_arr_tile_definitions_size - 1);
    SDL_LogDebug(LOG_CAT_MAPGEN, "Adding origin_coord hex to g_game_map.");

    // Center
    TileState* center_tile = malloc(sizeof(TileState));
    if (center_tile != NULL) {    
        center_tile->coord = origin_coord;
        center_tile->tile_def = g_arr_tile_definitions[rand_tile_def_id];
        center_tile->selected = false;
        center_tile->tile_biome = assign_biome(origin_coord);
        SDL_LogDebug(LOG_CAT_MAPGEN, "Placing tile at (%lld, %lld, %lld)", origin_coord.pos_q, origin_coord.pos_r, origin_coord.pos_s);
        hashmap_set(g_game_map, center_tile);
    }
    
    // Spiraling States
    uint64_t curr_tile_id = 1;
    for (uint64_t curr_radius = 1; curr_radius <= map_hex_radius; curr_radius++){
        uint64_t hexes_in_ring = hex_count_in_ring(curr_radius);        

        for (uint64_t curr_ring_pos = 0; curr_ring_pos < hexes_in_ring; curr_ring_pos++){
            CubeCoord curr_coord = spiral_coords[curr_radius][curr_ring_pos];            

            rand_tile_def_id = determine_rand_val(0, g_arr_tile_definitions_size - 1);            
            
            TileState* new_tile = malloc(sizeof(TileState));
            if (new_tile != NULL) {                
                new_tile->coord = curr_coord;
                new_tile->tile_def = g_arr_tile_definitions[rand_tile_def_id];
                new_tile->selected = false;
                new_tile->tile_biome = assign_biome(curr_coord);
                SDL_LogDebug(LOG_CAT_MAPGEN, "Placing tile at (%lld, %lld, %lld)", curr_coord.pos_q, curr_coord.pos_r, curr_coord.pos_s);
                hashmap_set(g_game_map, new_tile);
            }
            curr_tile_id++;            
        }
    }
}


uint8_t assign_biome(CubeCoord coord){
    float x = (float)coord.pos_q;
    float y = (float)coord.pos_r;
    float z = (float)coord.pos_s;
    float scale = determine_rand_val(0, 100) / 100.0f;    
    float noise_val = stb_perlin_noise3(x*scale,y*scale,z*scale,0,0,0);

    SDL_LogTrace(LOG_CAT_MAPGEN, "Got Noise: %.04f", noise_val);
    float norm = (noise_val + 1.0f) / 2.0f;

    uint8_t biome = BIOME_ARCTIC_TUNDRA;
    if (norm < 0.1f){
        biome = BIOME_FRESHWATER;
    }
    else if (norm < 0.2f){
        biome = BIOME_MARINE;
    }
    else if (norm < 0.3f){
        biome = BIOME_TROPICAL_GRASSLAND;
    }
    else if (norm < 0.4f){
        biome = BIOME_TEMPERATE_GRASSLAND;
    }
    else if (norm < 0.5f){
        biome = BIOME_TEMPERATE_RAINFOREST;
    }
    else if (norm < 0.6f){
        biome = BIOME_TROPICAL_RAINFOREST;
    }
    else if (norm < 0.7f){
        biome = BIOME_BOREAL_FOREST;
    }
    else if (norm < 0.8f){
        biome = BIOME_DESERT;
    }        
    else if (norm < 0.9f){
        biome = BIOME_ARCTIC_TUNDRA;
    }
    else{
        biome = BIOME_ALPINE_TUNDRA;
    }
    SDL_LogTrace(LOG_CAT_MAPGEN, "Assigned %s to [%"PRId64"][%"PRId64"][%"PRId64"].", get_biome_name(biome), coord.pos_q, coord.pos_r, coord.pos_s);
    return biome;
}

