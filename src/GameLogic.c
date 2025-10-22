#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <sys/time.h>
#include <SDL3/SDL.h>

#include "Globals.h"
#include "Util.h"
#include "cJSON.h"
#include "GameLogic.h"

void claim_tile(CubeCoord* tile_to_claim, uint8_t player_id){
	const TileState* selected_tile_state = hashmap_get(g_game_map, &(TileState){.coord = *tile_to_claim});
	if (!selected_tile_state) {
		SDL_LogError(LOG_CAT_DISPLAY, "No Tile State found for coords [%"PRId64"][%"PRId64"][%"PRId64"]", 
			tile_to_claim->pos_q, tile_to_claim->pos_r, tile_to_claim->pos_s
		);
		return;
	}
	
	if (selected_tile_state->pop_unit.pop_count == 0){
		return;
	}
	
	TileState new_state = *selected_tile_state;
	new_state.owned_by = OWNER_PLAYER;
	hashmap_set(g_game_map, &new_state);
}

void update_game_state(){
	
	// TODO calculate growth factor based on need fulfillment 
	g_total_world_pop = 0;
	
	// Spiraling States
	for (uint64_t curr_radius = 0; curr_radius <= HEX_GRID_RADIUS; curr_radius++){
        uint64_t hexes_in_ring = hex_count_in_ring(curr_radius);

        for (uint64_t curr_ring_pos = 0; curr_ring_pos < hexes_in_ring; curr_ring_pos++){
            CubeCoord curr_coord = g_game_map_spiral_coords[curr_radius][curr_ring_pos];
			
			const TileState* curr_tile_state = hashmap_get(g_game_map, &(TileState){.coord = curr_coord});
			if (!curr_tile_state) {
				SDL_LogError(LOG_CAT_DISPLAY, "No Tile State found for coords [%"PRId64"][%"PRId64"][%"PRId64"]", 
					curr_coord.pos_q, curr_coord.pos_r, curr_coord.pos_s
				);
				return;
			}

			if (curr_tile_state->tile_biome != BIOME_MARINE){
				TileState new_state = *curr_tile_state;
				new_state.pop_unit.pop_count++;
				hashmap_set(g_game_map, &new_state);
				g_total_world_pop += new_state.pop_unit.pop_count;
			}			
        }
    }
}