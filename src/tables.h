#ifndef TABLES_H
#define TABLES_H

#include "config.h"
#include "fixed.h"

/* Sine table (256 angles = 360 deg), 8.8 fixed point */
extern const signed int sin_table[256];

#define COS_LOOKUP(a) (sin_table[((unsigned char)((a) + 64))])
#define SIN_LOOKUP(a) (sin_table[(unsigned char)(a)])

/* Delta distances for DDA raycasting */
extern const unsigned int delta_x_table[256];
#define DELTA_X_LOOKUP(a) (delta_x_table[(unsigned char)(a)])
#define DELTA_Y_LOOKUP(a) (delta_x_table[((unsigned char)((a) - 64))])

/* Ray angle offsets and fish-eye cosine for NUM_RAYS (20 rays) */
extern const signed char ray_angle_offset[NUM_RAYS];
extern const unsigned char cos_ray_table[NUM_RAYS];

/* Projected wall height lookup (0..25 rows) */
extern const unsigned char height_table[256];

/* Vertical texture step lookup: (8 << 8) / wall_h */
extern const unsigned int v_step_table[26];

/* Precomputed Wolfenstein 3D Wall Textures: 5 types x 8 rows x 4 cols */
extern const unsigned char tex_chars[5][8][4];
extern const unsigned char tex_colors[5][8][4];

/* Directional lighting: shaded darker color for Y-side walls */
extern const unsigned char dark_colors[16];

/* Distance fog color table */
extern const unsigned char fog_colors[16];

#endif /* TABLES_H */
