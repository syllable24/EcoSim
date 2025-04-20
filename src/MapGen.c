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
    
    CubeCoord origin_coord = {0,0,0};
    CubeCoord** spiral_coords = cube_sprial(origin_coord, map_hex_radius);

    uint32_t rand_tile_def_id = determine_rand_val(0, g_arr_tile_definitions_size - 1);    

    // Center
    TileState* center_tile = malloc(sizeof(TileState));
    if (center_tile != NULL) {    
        center_tile->coord = origin_coord;
        center_tile->tile_def = g_arr_tile_definitions[rand_tile_def_id];
        center_tile->selected = false;
        center_tile->tile_biome = 0;        
        assign_biome(center_tile);
        SDL_LogTrace(LOG_CAT_MAPGEN, "Placing tile at (%lld, %lld, %lld)", origin_coord.pos_q, origin_coord.pos_r, origin_coord.pos_s);        
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
                new_tile->selected = false;
                new_tile->tile_biome = 0;
                
                new_tile->tile_def = malloc(sizeof (TileDefinition));
                if (new_tile->tile_def != NULL) {
                    *(new_tile->tile_def) = *(g_arr_tile_definitions[rand_tile_def_id]);
                }                
                
                assign_biome(new_tile);
                SDL_LogTrace(LOG_CAT_MAPGEN, "Placing tile at (%lld, %lld, %lld)", curr_coord.pos_q, curr_coord.pos_r, curr_coord.pos_s);
                hashmap_set(g_game_map, new_tile);
            }
            curr_tile_id++;            
        }
    }

    // Place random mountain seeds
    // Walk into random directions for random amount of tiles to from mountain chains, base on configurable world_mountain_precent

    // Place random marine seeds
    // Walk into random directions for random amount of tiles to from oceans, base on configurable world_marine_precent
}


uint8_t assign_biome(TileState* state){
    float x = (float)state->coord.pos_q;
    float y = (float)state->coord.pos_r;
    float z = (float)state->coord.pos_s;    
    float scale = determine_rand_val(0, 100) / 100.0f;    

    float temperature = (state->coord.pos_r + HEX_GRID_RADIUS / 2.0f) / HEX_GRID_RADIUS;
    temperature += fbm_noise(state->coord, scale, 3) * 0.2f;
    temperature = (temperature + 1.0f) / 2.0f;

    float moisture = fbm_noise(state->coord, scale, 4);
    moisture = (moisture + 1.0f) / 2.0f;

    float soil_quality = 0;
    float water_quality = 0;
    float air_quality = 0;

    uint8_t biome = BIOME_ARCTIC_TUNDRA;
    if (temperature > 0.66f) {
        if (moisture < 0.4f){ 
            biome = BIOME_DESERT;
            soil_quality  = roundf(determine_rand_val(0, 5)) / 100.0f;
            water_quality = roundf(determine_rand_val(0, 1)) / 100.0f;
            air_quality   = roundf(determine_rand_val(0, 10)) / 100.0f;
        }
        else if (moisture < 0.5f) {
            biome = BIOME_TROPICAL_GRASSLAND;
            soil_quality  = roundf(determine_rand_val(80, 100)) / 100.0f;
            water_quality = roundf(determine_rand_val(70, 100)) / 100.0f;
            air_quality   = roundf(determine_rand_val(70, 100)) / 100.0f;
        }
        else {
            biome = BIOME_TROPICAL_RAINFOREST;
            soil_quality  = roundf(determine_rand_val(40, 60)) / 100.0f;
            water_quality = roundf(determine_rand_val(30, 50)) / 100.0f;
            air_quality   = roundf(determine_rand_val(20, 70)) / 100.0f;
        }
    }
    else if (temperature > 0.33f) {
        if (moisture < 0.4f) {
            biome = BIOME_TEMPERATE_GRASSLAND;
            soil_quality  = roundf(determine_rand_val(80, 100)) / 100.0f;
            water_quality = roundf(determine_rand_val(70, 100)) / 100.0f;
            air_quality   = roundf(determine_rand_val(70, 100)) / 100.0f;
        }
        else if (moisture < 0.5f) {
            biome = BIOME_TEMPERATE_RAINFOREST;
            soil_quality  = roundf(determine_rand_val(50, 90)) / 100.0f;
            water_quality = roundf(determine_rand_val(80, 100)) / 100.0f;
            air_quality   = roundf(determine_rand_val(90, 100)) / 100.0f;            
        }
        else {
            biome = BIOME_BOREAL_FOREST;
            soil_quality  = roundf(determine_rand_val(60, 80)) / 100.0f;
            water_quality = roundf(determine_rand_val(40, 70)) / 100.0f;
            air_quality   = roundf(determine_rand_val(50, 80)) / 100.0f;            
        }
    }
    else {
        if (moisture < 0.5f) {
            biome = BIOME_ARCTIC_TUNDRA;
            soil_quality  = roundf(determine_rand_val(0, 10)) / 100.0f;
            water_quality = roundf(determine_rand_val(40, 60)) / 100.0f;
            air_quality   = roundf(determine_rand_val(60, 100)) / 100.0f;
        }
        else {
            biome = BIOME_ALPINE_TUNDRA;
            soil_quality = determine_rand_val(0, 5) / 100.0f;            
            water_quality = determine_rand_val(70, 100) / 100.0f;            
            air_quality = determine_rand_val(80, 100) / 100.0f;
        }
    }    
    state->tile_biome = biome;     
    state->tile_def->base_moisture = roundf(moisture * 100.0f) / 100.0f;
    state->tile_def->base_temperature = roundf(temperature * 100.0f) / 100.0f;

    state->tile_def->base_soil_quality = soil_quality;
    state->tile_def->base_water_quality = water_quality;
    state->tile_def->base_air_quality = air_quality;

    SDL_LogTrace(LOG_CAT_MAPGEN, "Assigned %s to [%"PRId64"][%"PRId64"][%"PRId64"].", get_biome_name(biome), 
        state->coord.pos_q, state->coord.pos_r, state->coord.pos_s
    );
    SDL_LogTrace(LOG_CAT_MAPGEN, "soil quality: [%.02f].", soil_quality);
    SDL_LogTrace(LOG_CAT_MAPGEN, "water quality: [%.02f].", water_quality);
    SDL_LogTrace(LOG_CAT_MAPGEN, "air quality: [%.02f].", air_quality);
    return biome;
}

float fbm_noise(CubeCoord coord, float scale, int octaves) {
    float total = 0.0f;
    float frequency = 1.0f;
    float amplitude = 1.0f;
    float max_value = 0.0f;

    for (int i = 0; i < octaves; i++) {
        total += stb_perlin_noise3(coord.pos_q * scale * frequency,
                                   coord.pos_r * scale * frequency,
                                   coord.pos_s * scale * frequency,
                                   0, 0, 0) * amplitude;

        max_value += amplitude;
        amplitude *= 0.5f;
        frequency *= 2.0f;
    }

    return total / max_value; // Normalize to [-1, 1]
}