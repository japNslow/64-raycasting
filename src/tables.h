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

/* Ray angle offsets and fish-eye cosine for 40 rays across 160 pixels */
extern const signed char ray_angle_offset[NUM_RAYS];
extern const unsigned char cos_ray_table[NUM_RAYS];

/* Projected wall height lookup (0..200 scanlines) */
extern const unsigned char height_table[256];

/* Vertical texture step lookup: (16 << 8) / wall_h */
extern const unsigned int v_step_table[201];

/* 160x200 Multicolor Bitmap Textures: 5 types x 16 scanlines */
extern const unsigned char bmp_textures[5][16];

/* Color palette per tile */
extern const unsigned char tile_color_screen[5];
extern const unsigned char tile_color_screen_dark[5];
extern const unsigned char tile_color_ram[5];

#endif /* TABLES_H */
