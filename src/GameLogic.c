#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <sys/time.h>
#include <SDL3/SDL.h>

#include "../include/Globals.h"
#include "../include/Util.h"
#include "../include/cJSON.h"

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