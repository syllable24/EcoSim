#include <stdlib.h>
#include <math.h>
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
    SDL_LogTrace(LOG_CAT_HEXMATH, "Calculated Hex Amount in ring radius: %d: %d.", radius, hex_amount);
    CubeCoord* ring = malloc(hex_amount * sizeof(CubeCoord));
    if (!ring){
        SDL_LogError(LOG_CAT_HEXMATH, "Memory allocation for hex ring failed (%u entries, %zu bytes).", hex_amount, hex_amount * sizeof(CubeCoord));        
        ring = NULL;
        return NULL;
    }
    CubeCoord hex = cube_add(orig, cube_scale(cube_direction_vectors[4], radius));

    uint64_t ring_index = 0;    
    for (uint8_t i = 0; i < 6; i++){
        for (uint8_t j = 0; j < radius; j++){            
            SDL_LogTrace(LOG_CAT_HEXMATH, "Ring radius %d: Coords: [%d][%d][%d].", 
                radius,                 
                hex.pos_q,hex.pos_r,hex.pos_s
            );
            ring[ring_index++] = hex;
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
    SDL_LogTrace(LOG_CAT_HEXMATH, "Calculated Hex Amount in Spiral: %d.", hex_amount);
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
        CubeCoord* ring = cube_ring(orig, spiral_index);                
        if (!ring){
            for (uint16_t j = 0; j < i; j++){
                free(spiral[j]);
                spiral[j] = NULL;
            }
            return NULL;
        }
        for (int j = 0; j < hex_count_in_ring(radius); j++){
            SDL_LogTrace(LOG_CAT_HEXMATH, "cube_spiral Ring radius %d: Hex Coords: [%d][%d][%d].", 
                radius,                 
                ring[j].pos_q, ring[j].pos_r, ring[j].pos_s
            );
        }
        

        spiral[spiral_index] = ring;
        spiral_index++;
    }
    return spiral;
}

SDL_FPoint flat_top_hex_to_pixel(CubeCoord coord, float hex_radius){    
    SDL_LogTrace(LOG_CAT_HEXMATH, "Converting Hex Coords: %s with radius %d.",
        print_coord(coord), hex_radius
    );

    float q = (float)coord.pos_q;
    float r = (float)coord.pos_r;

    float x = hex_radius * (1.5f * q);
    float y = hex_radius * (sqrtf(3.0f) * (r + q / 2.0f));

    SDL_LogTrace(LOG_CAT_HEXMATH, "Pixel position: x = %.2f, y = %.2f", x, y);

    return (SDL_FPoint){ x, y };
}

CubeCoord cube_round(float q, float r, float s) {
    int rq = roundf(q);
    int rr = roundf(r);
    int rs = roundf(s);

    float dq = fabsf(rq - q);
    float dr = fabsf(rr - r);
    float ds = fabsf(rs - s);

    if (dq > dr && dq > ds)
        rq = -rr - rs;
    else if (dr > ds)
        rr = -rq - rs;
    else
        rs = -rq - rr;

    CubeCoord result = { rq, rr, rs };
    return result;
}

CubeCoord flat_top_pixel_to_hex(SDL_FPoint point, float hex_radius){
    float frac_q = ((2.0f / 3.0f) * point.x) / hex_radius;
    float frac_r = ((-1.0f / 3.0f) * point.x + (sqrtf(3.0f) / 3.0f) * point.y) / hex_radius;
    float axial_s = -frac_q - frac_r;

    SDL_LogDebug(LOG_CAT_HEXMATH, "Pixel to hex: x = %.2f, y = %.2f", point.x, point.y);
    SDL_LogDebug(LOG_CAT_HEXMATH, "Pixel to hex: frac_q: (2.0f / 3.0f) * %.2f / %.2f = %.2f", point.x, hex_radius, frac_q);
    SDL_LogDebug(LOG_CAT_HEXMATH, "Pixel to hex: frac_r: (-1.0f / 3.0f) * %.2f / sqrtf(3.0f) * %.2f / %.2f = %.2f", point.x, point.y, hex_radius, frac_r);

    CubeCoord hex = cube_round(frac_q, frac_r, axial_s);
    return hex;
}