#include <stdlib.h>
#include <SDL3/SDL.h>


#include "../include/HexGridMath.h"
#include "../include/Util.h"

CubeCoord cube_direction_vectors[] = {
//  {  q,  r,  s }    
    {  1,  0, -1 }, // Bottom right
    {  1, -1,  0 }, // Top right
    {  0, -1,  1 }, // Top
    { -1,  0,  1 }, // Top left
    { -1,  1,  0 }, // Bottom left 
    {  0,  1, -1 }  // Bottom
};

CubeCoord axial_to_cube(AxialCoord axial_coord){    
    // Axial Coord System >> q + r + s = 0
    CubeCoord coord = {
        axial_coord.pos_q,
        axial_coord.pos_r,
        -axial_coord.pos_q - axial_coord.pos_r
    };
    return coord;
}

CubeCoord cube_add(CubeCoord orig, CubeCoord vect){    
    CubeCoord result = {
        orig.pos_q + vect.pos_q,
        orig.pos_r + vect.pos_r,
        orig.pos_s + vect.pos_s
    };
    return result;
}

CubeCoord cube_neighbor(CubeCoord orig, uint8_t direction){    
    return cube_add(orig, cube_direction_vectors[direction]);    
}

CubeCoord cube_scale(CubeCoord orig, uint16_t factor){
    CubeCoord result = {
        orig.pos_q * factor,
        orig.pos_r * factor,
        orig.pos_s * factor
    };
    return result;
}

uint64_t hex_count_in_ring(uint16_t radius){
    return 6 * radius;
}

CubeCoord* cube_ring(CubeCoord orig, uint16_t radius){
    uint64_t hex_amount = hex_count_in_ring(radius);
    CubeCoord* ring = malloc(hex_amount * sizeof(CubeCoord));
    if (!ring){
        SDL_LogError(LOG_CAT_HEXMATH, "Memory allocation for hex ring failed (%u entries, %zu bytes).", hex_amount, hex_amount * sizeof(CubeCoord));
        free(ring);
        ring = NULL;
        return NULL;
    }
    CubeCoord hex = cube_add(orig, cube_scale(cube_direction_vectors[4], radius));
    uint64_t ring_index = 0;
    for (uint8_t i = 0; i < 6; i++){
        for (uint8_t j = 0; j < radius; j++){
            ring[ring_index] = hex;
            hex = cube_neighbor(hex, i);
        }   
    }
    return ring;
}

uint64_t hex_count_in_sprial(uint16_t radius){
    return 1 + (3 * radius * (radius + 1));
}

CubeCoord** cube_sprial(CubeCoord orig, uint16_t radius){
    uint64_t hex_amount = hex_count_in_sprial(radius);
    CubeCoord** spiral = malloc((1 + radius) * sizeof(CubeCoord*));
    if (!spiral){
        SDL_LogError(LOG_CAT_HEXMATH, "Memory allocation for hex spiral failed (%u entries, %zu bytes).", (1 + radius), (1 + radius) * sizeof(CubeCoord*));        
        free(spiral);
        spiral = NULL;
        return NULL;
    }

    spiral[0] = &orig;
    uint64_t spiral_index = 1;
    for (uint16_t i = 0; i < radius; i++){
        CubeCoord* ring = cube_ring(orig, i);
        spiral[spiral_index] = ring;
        spiral_index++;
    }
    return spiral;
}