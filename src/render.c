#include "render.h"
#include "raycast.h"
#include "map.h"
#include "tables.h"

static unsigned char* screen_rows[SCREEN_ROWS];
static unsigned char* color_rows[SCREEN_ROWS];

/* Convert ASCII char to C64 VIC-II Screen Code */
static unsigned char ascii_to_screencode(char c) {
    if (c >= 'A' && c <= 'Z') return (unsigned char)(c - 64);
    if (c >= 'a' && c <= 'z') return (unsigned char)(c - 96);
    if (c == '@') return 0;
    return (unsigned char)c;
}

/* Print string directly to screen and color RAM */
static void print_at(unsigned char x, unsigned char y, const char* str, unsigned char color) {
    unsigned char* s = screen_rows[y] + x;
    unsigned char* c = color_rows[y] + x;
    while (*str) {
        *s++ = ascii_to_screencode(*str++);
        *c++ = color;
    }
}

void init_renderer(void) {
    unsigned char y;
    for (y = 0; y < SCREEN_ROWS; ++y) {
        screen_rows[y] = SCREEN_RAM + ((unsigned int)y * SCREEN_COLS);
        color_rows[y] = COLOR_RAM + ((unsigned int)y * SCREEN_COLS);
    }

    VIC_BORDER = C64_BLACK;
    VIC_BG = C64_BLACK;
}

void render_frame(void) {
    unsigned char x;

    for (x = 0; x < SCREEN_COLS; ++x) {
        unsigned char h = ray_hits[x].height;
        unsigned char tile = ray_hits[x].tile;
        unsigned char side = ray_hits[x].side;
        unsigned char dist = ray_hits[x].dist;
        unsigned char u = ray_hits[x].tex_u;
        unsigned char half_h = h >> 1;
        signed char top = HALF_VIEW_HEIGHT - half_h;
        signed char bot = HALF_VIEW_HEIGHT + half_h + (h & 1);
        unsigned char wall_h;
        signed char y;

        if (top < 0) top = 0;
        if (bot > VIEW_HEIGHT) bot = VIEW_HEIGHT;
        wall_h = (unsigned char)(bot - top);

        /* 1. Ceiling (Rows 0 to top-1) with distance stippling */
        for (y = 0; y < top; ++y) {
            if (y > 7 && ((x + y) & 1)) {
                screen_rows[y][x] = TEX_DOT;
                color_rows[y][x] = C64_DARKGRAY;
            } else {
                screen_rows[y][x] = TEX_SPACE;
                color_rows[y][x] = C64_BLACK;
            }
        }

        /* 2. Wall with Textures (Rows top to bot-1) */
        if (wall_h > 0) {
            for (y = top; y < bot; ++y) {
                unsigned char v = (unsigned char)(((unsigned int)(y - top) << 3) / wall_h);
                unsigned char ch;
                unsigned char col;

                if (v > 7) v = 7;

                switch (tile) {
                    case TILE_RED_BRICK:
                        /* Brick pattern: staggered mortar joints */
                        if (v == 0 || v == 4) {
                            /* Horizontal mortar line */
                            ch = TEX_HLINE;
                            col = (side == 0) ? C64_WHITE : C64_LIGHTGRAY;
                        } else if ((v < 4 && u == 0) || (v >= 4 && u == 4)) {
                            /* Vertical staggered mortar */
                            ch = TEX_VLINE;
                            col = (side == 0) ? C64_WHITE : C64_LIGHTGRAY;
                        } else {
                            /* Brick face */
                            ch = TEX_SOLID;
                            col = (side == 0) ? C64_LIGHTRED : C64_RED;
                        }
                        break;

                    case TILE_GREY_STONE:
                        /* Stone blocks with 3D beveled edges */
                        if (v == 0 || u == 0) {
                            /* Highlight edge */
                            ch = (v == 0) ? TEX_HLINE : TEX_VLINE;
                            col = (side == 0) ? C64_WHITE : C64_LIGHTGRAY;
                        } else if (v == 7 || u == 7) {
                            /* Shadow edge */
                            ch = (v == 7) ? TEX_HLINE : TEX_VLINE;
                            col = C64_DARKGRAY;
                        } else {
                            /* Stone interior */
                            ch = (dist > 60) ? TEX_DITHER1 : TEX_SOLID;
                            col = (side == 0) ? C64_LIGHTGRAY : C64_GRAY;
                        }
                        break;

                    case TILE_BLUE_STONE:
                        /* Wolfenstein blue stone with golden emblem */
                        if (v == 0 || v == 7 || u == 0 || u == 7) {
                            /* Frame border */
                            ch = (v == 0 || v == 7) ? TEX_HLINE : TEX_VLINE;
                            col = (side == 0) ? C64_BLUE : C64_BLACK;
                        } else if ((u == 3 || u == 4) && (v == 3 || v == 4)) {
                            /* Golden cross/emblem */
                            ch = TEX_CROSS;
                            col = C64_YELLOW;
                        } else {
                            /* Blue stone face */
                            ch = (dist > 60) ? TEX_DITHER1 : TEX_SOLID;
                            col = (side == 0) ? C64_LIGHTBLUE : C64_BLUE;
                        }
                        break;

                    case TILE_WOOD:
                        /* Vertical wooden planks */
                        if (u == 0 || u == 4) {
                            ch = TEX_VLINE;
                            col = C64_BROWN;
                        } else if (v == 0 || v == 7) {
                            ch = TEX_HLINE;
                            col = C64_BROWN;
                        } else {
                            ch = TEX_SOLID;
                            col = (side == 0) ? C64_ORANGE : C64_BROWN;
                        }
                        break;

                    case TILE_DOOR:
                        /* Metal door with frame, panels, and gold handle */
                        if (u == 0 || u == 7) {
                            ch = TEX_VLINE;
                            col = C64_LIGHTGRAY;
                        } else if (v == 0) {
                            ch = TEX_HLINE;
                            col = C64_LIGHTGRAY;
                        } else if (u == 5 && v == 4) {
                            /* Brass doorknob */
                            ch = TEX_KNOB;
                            col = C64_YELLOW;
                        } else if ((v == 2 || v == 6) && (u >= 2 && u <= 5)) {
                            ch = TEX_HLINE;
                            col = C64_GRAY;
                        } else {
                            ch = TEX_SOLID;
                            col = (side == 0) ? C64_GRAY : C64_DARKGRAY;
                        }
                        break;

                    default:
                        ch = TEX_SOLID;
                        col = C64_GRAY;
                        break;
                }

                /* Distance fogging for distant walls */
                if (dist > 85) {
                    ch = TEX_DITHER1;
                    col = (side == 0) ? C64_DARKGRAY : C64_BLACK;
                }

                screen_rows[y][x] = ch;
                color_rows[y][x] = col;
            }
        }

        /* 3. Floor (Rows bot to VIEW_HEIGHT-1) with perspective lines */
        for (y = bot; y < VIEW_HEIGHT; ++y) {
            if (y > 21) {
                /* Close foreground floor tiles */
                if ((x & 3) == 0) {
                    screen_rows[y][x] = TEX_VLINE;
                    color_rows[y][x] = C64_GRAY;
                } else {
                    screen_rows[y][x] = TEX_FLOOR_LINE;
                    color_rows[y][x] = C64_BROWN;
                }
            } else if (y > 17) {
                screen_rows[y][x] = TEX_FLOOR_LINE;
                color_rows[y][x] = C64_DARKGRAY;
            } else {
                screen_rows[y][x] = TEX_DOT;
                color_rows[y][x] = C64_BLACK;
            }
        }
    }
}

void render_fullscreen_map(void) {
    unsigned char x, y;
    signed char px = (signed char)(player_x >> FP_SHIFT);
    signed char py = (signed char)(player_y >> FP_SHIFT);

    /* Clear screen to black */
    for (y = 0; y < SCREEN_ROWS; ++y) {
        for (x = 0; x < SCREEN_COLS; ++x) {
            screen_rows[y][x] = TEX_SPACE;
            color_rows[y][x] = C64_BLACK;
        }
    }

    print_at(9, 1, "=== 3D TACTICAL MAP ===", C64_YELLOW);
    print_at(5, 2, "PLAYER: (O)  WALL: [3D]  DOOR: (=)", C64_CYAN);

    /* Render 16x16 map with 3D block relief */
    for (y = 0; y < MAP_HEIGHT; ++y) {
        for (x = 0; x < MAP_WIDTH; ++x) {
            unsigned char sx = 12 + x;
            unsigned char sy = 4 + y;
            unsigned char t = game_map[y][x];

            if ((signed char)x == px && (signed char)y == py) {
                /* Player position marker */
                screen_rows[sy][sx] = TEX_KNOB;
                color_rows[sy][sx] = C64_WHITE;
            } else if (t == TILE_DOOR) {
                screen_rows[sy][sx] = '=';
                color_rows[sy][sx] = C64_YELLOW;
            } else if (t != TILE_EMPTY) {
                /* 3D block top face */
                screen_rows[sy][sx] = TEX_SOLID;
                switch (t) {
                    case TILE_BLUE_STONE: color_rows[sy][sx] = C64_LIGHTBLUE; break;
                    case TILE_RED_BRICK:  color_rows[sy][sx] = C64_LIGHTRED; break;
                    case TILE_WOOD:       color_rows[sy][sx] = C64_ORANGE; break;
                    default:              color_rows[sy][sx] = C64_LIGHTGRAY; break;
                }
            } else {
                /* Empty floor tile with subtle dot */
                screen_rows[sy][sx] = TEX_DOT;
                color_rows[sy][sx] = C64_DARKGRAY;
            }
        }
    }

    /* Draw player view direction ray on map */
    {
        signed char dir_dx = (signed char)(COS_LOOKUP(player_angle) >> 6);
        signed char dir_dy = (signed char)(SIN_LOOKUP(player_angle) >> 6);
        signed char fx = px + (dir_dx > 0 ? 1 : (dir_dx < 0 ? -1 : 0));
        signed char fy = py + (dir_dy > 0 ? 1 : (dir_dy < 0 ? -1 : 0));
        if (fx >= 0 && fx < MAP_WIDTH && fy >= 0 && fy < MAP_HEIGHT) {
            if (game_map[(unsigned char)fy][(unsigned char)fx] == TILE_EMPTY) {
                screen_rows[4 + fy][12 + fx] = TEX_CROSS;
                color_rows[4 + fy][12 + fx] = C64_LIGHTGREEN;
            }
        }
    }

    print_at(6, 21, "[W/A/S/D] MOVE/TURN  [SPACE] DOOR", C64_GRAY);
    print_at(9, 23, "PRESS [M] TO RETURN TO 3D", C64_GREEN);
}
