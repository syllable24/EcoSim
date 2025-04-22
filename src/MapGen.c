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
    return hashmap_sip((const CubeCoord*)rec, sizeof(CubeCoord), seed0, seed1);
}

void generate_mountain_chain(CubeCoord* arr_mountain_coords, uint16_t* arr_mountain_coords_size){
    bool valid = false;
    
    CubeCoord seed_coord = (CubeCoord){
        0,0,0
    };
    
    pick_seed_tile(&seed_coord, BIOME_MOUNTAIN);
    const TileState* tile_state = hashmap_get(g_game_map, &(TileState){.coord=seed_coord});
    if (tile_state == NULL) {
        SDL_LogDebug(LOG_CAT_MAPGEN, "Got NULL tile_state at (%d, %d, %d)", 
            seed_coord.pos_q , seed_coord.pos_r, seed_coord.pos_s
        );
        return;
    }

    hashmap_set(g_game_map, &(TileState){
        .coord=tile_state->coord,
        .tile_def=tile_state->tile_def,
        .selected=tile_state->selected,
        .tile_biome=BIOME_MOUNTAIN
    });

    arr_mountain_coords[*arr_mountain_coords_size] = tile_state->coord;
    (*arr_mountain_coords_size)++;

    CubeCoord curr_coords = tile_state->coord;
    uint8_t reject_counter = 0;
    uint8_t chain_length = determine_rand_val(MOUNTAIN_CHAIN_MIN_LENGTH, MOUNTAIN_CHAIN_MAX_LENGTH);
    for (int i = 0; i < chain_length; i++){
        uint8_t direction = determine_rand_val(0, 6);
        CubeCoord neighbor = cube_neighbor(curr_coords, direction);
        if (reject_counter == 6){
            // Abort Mountain Chain
            break;
        }
        if (tile_state == NULL) {
            SDL_LogDebug(LOG_CAT_MAPGEN, "Got NULL tile_state at (%d, %d, %d)", 
                curr_coords.pos_q , curr_coords.pos_r, curr_coords.pos_s
            );
            reject_counter++;
            i--;
            continue;
        }
        if (neighbor.pos_q > HEX_GRID_RADIUS || neighbor.pos_q < -HEX_GRID_RADIUS
            || neighbor.pos_r > HEX_GRID_RADIUS || neighbor.pos_r < -HEX_GRID_RADIUS
            || neighbor.pos_s > HEX_GRID_RADIUS || neighbor.pos_s < -HEX_GRID_RADIUS){
            // Reject position
            i--;
            continue;
        }
        const TileState* tile_state = hashmap_get(g_game_map, &(TileState){.coord=neighbor});
        if (tile_state->tile_biome == BIOME_MOUNTAIN){
            // Reject position
            reject_counter++;
            i--;
            continue;
        }

        // Accept 
        reject_counter = 0;
        curr_coords = neighbor;        
        hashmap_set(g_game_map, &(TileState){
            .coord=tile_state->coord,
            .tile_def=tile_state->tile_def,
            .selected=tile_state->selected,
            .tile_biome=BIOME_MOUNTAIN
        });
        arr_mountain_coords[*arr_mountain_coords_size] = tile_state->coord;
        (*arr_mountain_coords_size)++;
    }
}

const TileState* pick_seed_tile(CubeCoord* seed_coord, uint8_t filter){
    bool valid = false;    
    const TileState* tile_state = NULL;
    while(!valid){
        int16_t rand_q = determine_rand_val(-HEX_GRID_RADIUS, HEX_GRID_RADIUS);
        int16_t rand_r = determine_rand_val(-HEX_GRID_RADIUS, HEX_GRID_RADIUS);
        int16_t rand_s = (rand_q * -1) - rand_r;
        if (rand_s > HEX_GRID_RADIUS || rand_s < -HEX_GRID_RADIUS){
            // Rolled out of bounds seed
            continue;
        }

        *(seed_coord) = (CubeCoord){
            rand_q, 
            rand_r, 
            rand_s        
        };
        SDL_LogTrace(LOG_CAT_MAPGEN, "Determined Seed (%d, %d, %d)", 
            seed_coord->pos_q , seed_coord->pos_r, seed_coord->pos_s
        );
        tile_state = hashmap_get(g_game_map, &(TileState){.coord=*(seed_coord)});
        if (tile_state == NULL) {
            SDL_LogError(LOG_CAT_MAPGEN, "Got NULL tile_state at (%d, %d, %d)", 
                seed_coord->pos_q , seed_coord->pos_r, seed_coord->pos_s
            );
        }
        if (tile_state != NULL && tile_state->tile_biome != filter) {
            valid = true;
        }
    }
    return tile_state;
}

void generate_river(CubeCoord* arr_mountain_coords, uint16_t arr_mountain_coords_size){
    CubeCoord river_seed = {0,0,0};
    bool valid = false;
    while (!valid){
        int32_t rand_index = determine_rand_val(0, arr_mountain_coords_size - 1);
        river_seed = arr_mountain_coords[rand_index];
        valid = !any_neighbor_matches(river_seed, is_marine, NULL);
    }
    SDL_LogTrace(LOG_CAT_MAPGEN, "Determined river seed [%lld][%lld][%lld]", river_seed.pos_q,river_seed.pos_r,river_seed.pos_s);
        
    const TileState* river_state = hashmap_get(g_game_map, &(TileState){.coord=river_seed});

    int32_t rand_length = determine_rand_val(RIVER_MIN_LENGTH, RIVER_MAX_LENGTH);

    CubeCoord curr_coord = river_state->coord;
    for(int i = 0; i < rand_length; i++){
        uint8_t rand_direction = determine_rand_val(0, 6);
        CubeCoord neighbor = cube_neighbor(curr_coord, rand_direction);

        
    }

}

void generate_marine_chain(){
    bool seed_valid = false;
    bool valid = false;
    
    CubeCoord seed_coord = (CubeCoord){
        0,0,0
    };
    const TileState* tile_state = NULL;

    while (!seed_valid){
        tile_state = pick_seed_tile(&seed_coord, BIOME_MARINE);         
        if (tile_state == NULL) {
            SDL_LogDebug(LOG_CAT_MAPGEN, "Got NULL tile_state at (%d, %d, %d)", 
                seed_coord.pos_q , seed_coord.pos_r, seed_coord.pos_s
            );
            return;
        }
    
        seed_valid = !any_neighbor_matches(tile_state->coord, is_marine_or_mountain, NULL);
    }

    hashmap_set(g_game_map, &(TileState){
        .coord=tile_state->coord,
        .tile_def=tile_state->tile_def,
        .selected=tile_state->selected,
        .tile_biome=BIOME_MARINE
    });

    // Set neighbors to Biome MARINE    
    for (int i = 0; i < 6; i++){
        CubeCoord neighbor = cube_neighbor(tile_state->coord, i);
        if (is_out_of_bounds(neighbor)){
            continue;
        }
        const TileState* neighbor_state = hashmap_get(g_game_map, &(TileState){.coord=neighbor});
        hashmap_set(g_game_map, &(TileState){
            .coord=neighbor_state->coord,
            .tile_def=neighbor_state->tile_def,
            .selected=neighbor_state->selected,
            .tile_biome=BIOME_MARINE
        });
    }
    
    CubeCoord curr_coords = tile_state->coord;
    uint8_t reject_counter = 0;
    uint8_t chain_length = determine_rand_val(MARINE_CHAIN_MIN_LENGTH, MARINE_CHAIN_MAX_LENGTH);
    for (int i = 0; i < chain_length; i++){
        uint8_t direction = determine_rand_val(0, 6);
        CubeCoord next_neighbor = cube_neighbor(cube_neighbor(cube_neighbor(curr_coords, direction), direction), direction);

        if (reject_counter == 6){
            // Abort Marine Chain
            break;
        }
        if (is_out_of_bounds(next_neighbor)){
            // Reject position
            i--;
            continue;
        }
        if (tile_state == NULL) {
            SDL_LogDebug(LOG_CAT_MAPGEN, "Got NULL tile_state at (%d, %d, %d)", 
                curr_coords.pos_q , curr_coords.pos_r, curr_coords.pos_s
            );
            reject_counter++;
            i--;
            continue;
        }
        const TileState* tile_state = hashmap_get(g_game_map, &(TileState){.coord=next_neighbor});        
        if (tile_state->tile_biome == BIOME_MOUNTAIN || tile_state->tile_biome == BIOME_MARINE){
            // Reject position
            reject_counter++;
            i--;
            continue;
        }

        // Accept 
        reject_counter = 0;
        curr_coords = next_neighbor;        
        hashmap_set(g_game_map, &(TileState){
            .coord=tile_state->coord,
            .tile_def=tile_state->tile_def,
            .selected=tile_state->selected,
            .tile_biome=BIOME_MARINE
        });        

        // Set neighbors to Biome MARINE
        for (int i = 0; i < 6; i++){
            CubeCoord neighbor = cube_neighbor(tile_state->coord, i);
            const TileState* neighbor_state = hashmap_get(g_game_map, &(TileState){.coord=neighbor});
            if (is_out_of_bounds(neighbor)){
                continue;
            }            
            if (neighbor_state->tile_biome == BIOME_MOUNTAIN || neighbor_state->tile_biome == BIOME_MARINE){                
                continue;
            }
            hashmap_set(g_game_map, &(TileState){
                .coord=neighbor_state->coord,
                .tile_def=neighbor_state->tile_def,
                .selected=neighbor_state->selected,
                .tile_biome=BIOME_MARINE
            });
        }

    }
}

// Assumes symmetrical hexagonal flat-top hex-grid. (Big hexagon composed of smaller hexagons)
int generate_map(uint8_t map_hex_radius){
    // Validate config and inputs
    if (!g_arr_tile_definitions || g_arr_tile_definitions_size <= 0 || map_hex_radius <= 0 ) {
        SDL_LogError(LOG_CAT_MAPGEN, "Invalid input: g_arr_tile_definitions=%p, g_arr_tile_definitions_size=%u, map_hex_radius=%u",
                     g_arr_tile_definitions, g_arr_tile_definitions_size, map_hex_radius);
        return SDL_APP_FAILURE;
    }

    generate_base_tiles(map_hex_radius);

    CubeCoord* arr_mountain_coords = malloc(MOUNTAIN_CHAIN_MAX_LENGTH * MOUNTAIN_CHAIN_MAX_AMOUNT * sizeof(CubeCoord));
    if (!arr_mountain_coords){
        SDL_LogError(LOG_CAT_MAPGEN, "Memory Allocation for arr_mountain_coords failed");        
        return SDL_APP_FAILURE;
    }
    uint16_t arr_mountain_coords_size = 0;

    // Place random mountain seeds
    // Walk into random directions for random amount of tiles to from mountain chains    
    uint8_t chain_amount = determine_rand_val(MOUNTAIN_CHAIN_MIN_AMOUNT, MOUNTAIN_CHAIN_MAX_AMOUNT);
    SDL_LogDebug(LOG_CAT_MAPGEN, "Start generating %d mountain chains", chain_amount);
    for (int i = 0; i < chain_amount; i++){
        generate_mountain_chain(arr_mountain_coords, &arr_mountain_coords_size);
    }

    // Place random marine seeds
    // Walk into random directions for random amount of 7-tile-groups to from oceans
    uint8_t marine_chain_amount = determine_rand_val(MOUNTAIN_CHAIN_MIN_AMOUNT, MOUNTAIN_CHAIN_MAX_AMOUNT);
    SDL_LogDebug(LOG_CAT_MAPGEN, "Start generating %d marine chains", chain_amount);
    for (int i = 0; i < marine_chain_amount; i++){
        generate_marine_chain();
    }

    // Place random river seeds on mountains
    // Walk into random directions until the river length is hit (then form a lage) or a marine tile is found.
    uint8_t river_amount = determine_rand_val(RIVER_MIN_AMOUNT, RIVER_MAX_AMOUNT);
    SDL_LogDebug(LOG_CAT_MAPGEN, "Start generating %d rivers", river_amount);
    for (int i = 0; i < river_amount; i++){
        generate_river(arr_mountain_coords, arr_mountain_coords_size);
    }

    return SDL_APP_CONTINUE;
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

void generate_base_tiles(uint8_t map_hex_radius){

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
        center_tile->has_river = false;
        center_tile->river_direction = (CubeCoord){0,0,0};
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
                new_tile->has_river = false;
                new_tile->river_direction = (CubeCoord){0,0,0};
                
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
}

bool any_neighbor_matches(CubeCoord center, NeighborPredicate predicate, void* context) {
    for (int i = 0; i < 6; i++) {    
        CubeCoord neighbor = cube_neighbor(center, i);
        SDL_LogTrace(LOG_CAT_MAPGEN, "Check Neighbor [%lld][%lld][%lld]", neighbor.pos_q,neighbor.pos_r,neighbor.pos_s);
        if (is_out_of_bounds(neighbor)){
            SDL_LogTrace(LOG_CAT_MAPGEN, "Neighbor [%lld][%lld][%lld] OOB", neighbor.pos_q,neighbor.pos_r,neighbor.pos_s);
            continue;
        }
        const TileState* neighbor_state = hashmap_get(g_game_map, &(TileState){.coord = neighbor});
        if (neighbor_state && predicate(neighbor_state, context)) {
            return true;
        }
    }
    return false;
}

bool is_out_of_bounds(CubeCoord coord){
    bool oob = coord.pos_q > HEX_GRID_RADIUS || coord.pos_q < -HEX_GRID_RADIUS
            || coord.pos_r > HEX_GRID_RADIUS || coord.pos_r < -HEX_GRID_RADIUS
            || coord.pos_s > HEX_GRID_RADIUS || coord.pos_s < -HEX_GRID_RADIUS;
    return oob;
}

bool is_marine_or_mountain(const TileState* tile, void* context) {
    return is_marine(tile, context) || is_mountain(tile, context);
}

bool is_marine(const TileState* tile, void* context) {
    return tile->tile_biome == BIOME_MARINE;
}

bool is_mountain(const TileState* tile, void* context) {
    return tile->tile_biome == BIOME_MOUNTAIN;
}

// End of file