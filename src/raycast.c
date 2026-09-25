#include "raycast.h"
#include "tables.h"
#include "map.h"

RayHit ray_hits[SCREEN_COLS];

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

    for (col = 0; col < SCREEN_COLS; ++col) {
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
        fixed hit_pos;

        ray_angle = (unsigned char)(player_angle + ray_angle_offset[col]);
        cos_val = COS_LOOKUP(ray_angle);
        sin_val = SIN_LOOKUP(ray_angle);

        delta_x = DELTA_X_LOOKUP(ray_angle);
        delta_y = DELTA_Y_LOOKUP(ray_angle);

        map_x = (signed char)(player_x >> FP_SHIFT);
        map_y = (signed char)(player_y >> FP_SHIFT);

        if (cos_val >= 0) {
            step_x = 1;
            side_dist_x = (unsigned int)(((unsigned long)(256 - (player_x & 0xFF)) * delta_x) >> 8);
        } else {
            step_x = -1;
            side_dist_x = (unsigned int)(((unsigned long)(player_x & 0xFF) * delta_x) >> 8);
        }

        if (sin_val >= 0) {
            step_y = 1;
            side_dist_y = (unsigned int)(((unsigned long)(256 - (player_y & 0xFF)) * delta_y) >> 8);
        } else {
            step_y = -1;
            side_dist_y = (unsigned int)(((unsigned long)(player_y & 0xFF) * delta_y) >> 8);
        }

        /* DDA stepping loop */
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

            if (map_x < 0 || map_x >= MAP_WIDTH || map_y < 0 || map_y >= MAP_HEIGHT) {
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
                hit_pos = player_y + (fixed)(((signed long)raw_dist * sin_val) >> 8);
            } else {
                raw_dist = side_dist_y - delta_y;
                hit_pos = player_x + (fixed)(((signed long)raw_dist * cos_val) >> 8);
            }

            /* Fish-eye correction */
            perp_dist = (unsigned int)(((unsigned long)raw_dist * cos_ray_table[col]) >> 8);
            d = perp_dist >> 4;
            if (d > 255) d = 255;

            ray_hits[col].height = height_table[(unsigned char)d];
            ray_hits[col].tile = game_map[(unsigned char)map_y][(unsigned char)map_x];
            ray_hits[col].side = side;
            ray_hits[col].dist = (unsigned char)d;
            ray_hits[col].tex_u = (unsigned char)((hit_pos & 0xFF) >> 5);
        } else {
            ray_hits[col].height = 1;
            ray_hits[col].tile = TILE_GREY_STONE;
            ray_hits[col].side = 0;
            ray_hits[col].dist = 255;
            ray_hits[col].tex_u = 0;
        }
    }
}
