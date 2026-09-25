#include "raycast.h"
#include "tables.h"
#include "map.h"

RayHit ray_hits[NUM_RAYS];

fixed player_x;
fixed player_y;
unsigned char player_angle;

void init_raycaster(void) {
    /* Spawn player at (3.5, 3.5), facing angle 0 (East) */
    player_x = INT_TO_FIXED(3) + FP_HALF;
    player_y = INT_TO_FIXED(3) + FP_HALF;
    player_angle = 0;
}

void raycast_all(void) {
    unsigned char col;

    for (col = 0; col < NUM_RAYS; ++col) {
        unsigned char ray_angle;
        signed int cos_val;
        signed int sin_val;
        unsigned int delta_x;
        unsigned int delta_y;
        signed char map_x;
        signed char map_y;
        signed char step_x;
        signed char step_y;
        unsigned int side_dist_x;
        unsigned int side_dist_y;
        unsigned char hit = 0;
        unsigned char side = 0;
        unsigned char steps = 0;
        unsigned int raw_dist;
        unsigned int perp_dist;
        unsigned int d;
        unsigned char frac_x;
        unsigned char frac_y;
        unsigned int dx_scaled;
        unsigned int dy_scaled;
        unsigned char u;

        ray_angle = (unsigned char)(player_angle + ray_angle_offset[col]);
        cos_val = COS_LOOKUP(ray_angle);
        sin_val = SIN_LOOKUP(ray_angle);

        delta_x = DELTA_X_LOOKUP(ray_angle);
        delta_y = DELTA_Y_LOOKUP(ray_angle);

        map_x = (signed char)(player_x >> FP_SHIFT);
        map_y = (signed char)(player_y >> FP_SHIFT);

        frac_x = (unsigned char)(player_x & 0xFF);
        frac_y = (unsigned char)(player_y & 0xFF);

        /* Fast 16-bit side_dist calculation without 32-bit math */
        if (cos_val >= 0) {
            step_x = 1;
            dx_scaled = (256 - frac_x) >> 2;
        } else {
            step_x = -1;
            dx_scaled = frac_x >> 2;
        }
        side_dist_x = (dx_scaled * (delta_x >> 2)) >> 4;

        if (sin_val >= 0) {
            step_y = 1;
            dy_scaled = (256 - frac_y) >> 2;
        } else {
            step_y = -1;
            dy_scaled = frac_y >> 2;
        }
        side_dist_y = (dy_scaled * (delta_y >> 2)) >> 4;

        /* Ultra-fast 16-bit DDA loop */
        while (!hit && steps < MAX_RAY_STEPS) {
            if (side_dist_x < side_dist_y) {
                side_dist_x += delta_x;
                map_x += step_x;
                side = 0;
            } else {
                side_dist_y += delta_y;
                map_y += step_y;
                side = 1;
            }

            if ((unsigned char)map_x >= MAP_WIDTH || (unsigned char)map_y >= MAP_HEIGHT) {
                break;
            }

            if (game_map[(unsigned char)map_y][(unsigned char)map_x] != TILE_EMPTY) {
                hit = 1;
            }
            steps++;
        }

        if (hit) {
            if (side == 0) {
                raw_dist = side_dist_x - delta_x;
                /* Fast texture u calculation */
                u = (unsigned char)((player_y + (((raw_dist >> 2) * (sin_val >> 2)) >> 4)) >> 6) & 3;
            } else {
                raw_dist = side_dist_y - delta_y;
                u = (unsigned char)((player_x + (((raw_dist >> 2) * (cos_val >> 2)) >> 4)) >> 6) & 3;
            }

            /* Fish-eye correction (16-bit) */
            perp_dist = ((raw_dist >> 4) * cos_ray_table[col]) >> 4;
            d = perp_dist >> 4;
            if (d > 255) d = 255;

            ray_hits[col].height = height_table[(unsigned char)d];
            ray_hits[col].tile = game_map[(unsigned char)map_y][(unsigned char)map_x];
            ray_hits[col].side = side;
            ray_hits[col].dist = (unsigned char)d;
            ray_hits[col].tex_u = u;
        } else {
            ray_hits[col].height = 1;
            ray_hits[col].tile = TILE_GREY_STONE;
            ray_hits[col].side = 0;
            ray_hits[col].dist = 255;
            ray_hits[col].tex_u = 0;
        }
    }
}
