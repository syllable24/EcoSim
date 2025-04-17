#include <string.h>

#ifndef HEXGRIDMATH_H
#define HEXGRIDMATH_H

typedef struct {
    const uint32_t pos_q;
    const uint32_t pos_r;
} AxialCoord;

typedef struct {
    uint64_t pos_q;
    uint64_t pos_r;
    uint64_t pos_s;
} CubeCoord;

CubeCoord axial_to_cube(AxialCoord axial_coord);

CubeCoord cube_add(CubeCoord orig, CubeCoord vect);

CubeCoord cube_neighbor(CubeCoord orig, uint8_t direction);

CubeCoord cube_scale(CubeCoord orig, uint16_t factor);

uint64_t hex_count_in_ring(uint16_t radius);
CubeCoord* cube_ring(CubeCoord orig, uint16_t radius);

uint64_t hex_count_in_sprial(uint16_t radius);
CubeCoord** cube_sprial(CubeCoord orig, uint16_t radius);

#endif

