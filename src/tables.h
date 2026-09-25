#ifndef TABLES_H
#define TABLES_H

#include "fixed.h"

/* Sine table (256 angles = 360 deg), 8.8 fixed point */
extern const signed int sin_table[256];

/* Cosine table lookup macro: cos(a) = sin((a + 64) & 255) */
#define COS_LOOKUP(a) (sin_table[((unsigned char)((a) + 64))])
#define SIN_LOOKUP(a) (sin_table[(unsigned char)(a)])

/* Delta distances for DDA raycasting */
extern const unsigned int delta_x_table[256];
#define DELTA_X_LOOKUP(a) (delta_x_table[(unsigned char)(a)])
#define DELTA_Y_LOOKUP(a) (delta_x_table[((unsigned char)((a) - 64))])

/* Ray angle offset from player view angle for each of the 40 screen columns */
extern const signed char ray_angle_offset[40];

/* Cosine of ray angle offset (used to eliminate fish-eye distortion) */
extern const unsigned char cos_ray_table[40];

/* Projected wall height (in character rows, 1..19) for distance d (0..255) */
extern const unsigned char height_table[256];

#endif /* TABLES_H */
