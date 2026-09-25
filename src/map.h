#ifndef MAP_H
#define MAP_H

#define MAP_WIDTH   16
#define MAP_HEIGHT  16

#define TILE_EMPTY      0
#define TILE_GREY_STONE 1
#define TILE_BLUE_STONE 2
#define TILE_WOOD       3
#define TILE_RED_BRICK  4
#define TILE_DOOR       5

extern unsigned char game_map[MAP_HEIGHT][MAP_WIDTH];

/* Check if a tile block is solid/impassable */
#define IS_SOLID(x, y) (game_map[(unsigned char)(y)][(unsigned char)(x)] != TILE_EMPTY)

/* Fast map cell lookup */
#define GET_TILE(x, y) (game_map[(unsigned char)(y)][(unsigned char)(x)])

#endif /* MAP_H */
