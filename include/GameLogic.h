#include <stdint.h>

#include "Globals.h"

#ifndef GAMELOGIC_H
#define GAMELOGIC_H

void claim_tile(CubeCoord* tile_to_claim, uint8_t player_id);

void update_game_state();

#endif