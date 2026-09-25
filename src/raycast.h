#ifndef RAYCAST_H
#define RAYCAST_H

#include "config.h"
#include "fixed.h"

typedef struct {
    unsigned char height;    /* Wall height in scanlines (0..200) */
    unsigned char tile;      /* Map tile type (1..5) */
    unsigned char side;      /* 0 = X side, 1 = Y side */
    unsigned char dist;      /* Perpendicular distance */
} RayHit;

extern RayHit ray_hits[NUM_RAYS];

/* Player state */
extern fixed player_x;
extern fixed player_y;
extern unsigned char player_angle;

void init_raycaster(void);
void raycast_all(void);

#endif /* RAYCAST_H */
