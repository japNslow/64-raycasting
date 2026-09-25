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
    unsigned char r;

    for (r = 0; r < NUM_RAYS; ++r) {
        unsigned char col_x = (unsigned char)(r << 1);
        unsigned char h = ray_hits[r].height;
        unsigned char tile_raw = ray_hits[r].tile;
        unsigned char tile_idx = (tile_raw > 0 && tile_raw <= 5) ? (tile_raw - 1) : 0;
        unsigned char side = ray_hits[r].side;
        unsigned char dist = ray_hits[r].dist;
        unsigned char u0 = ray_hits[r].tex_u;
        unsigned char u1 = (unsigned char)((u0 + 1) & 3);
        unsigned char half_h = h >> 1;
        signed char top = HALF_VIEW_HEIGHT - half_h;
        signed char bot = HALF_VIEW_HEIGHT + half_h + (h & 1);
        unsigned char wall_h;
        unsigned int v_step;
        unsigned int v_acc;
        unsigned char is_fog;
        unsigned char* scr;
        unsigned char* col;
        signed char y;

        if (top < 0) top = 0;
        if (bot > VIEW_HEIGHT) bot = VIEW_HEIGHT;
        wall_h = (unsigned char)(bot - top);

        /* Pointer to top of column pair (col_x and col_x + 1) */
        scr = SCREEN_RAM + col_x;
        col = COLOR_RAM + col_x;

        /* 1. Fast Ceiling (Rows 0 to top-1) */
        for (y = 0; y < top; ++y) {
            scr[0] = TEX_SPACE;
            scr[1] = TEX_SPACE;
            col[0] = C64_BLACK;
            col[1] = C64_BLACK;
            scr += 40;
            col += 40;
        }

        /* 2. Fast Textured Wall (Rows top to bot-1) */
        if (wall_h > 0) {
            v_step = v_step_table[wall_h];
            v_acc = 0;
            is_fog = (dist > 85);

            for (y = top; y < bot; ++y) {
                unsigned char v = (unsigned char)(v_acc >> 8);
                unsigned char ch0, ch1;
                unsigned char c0, c1;

                v_acc += v_step;
                if (v > 7) v = 7;

                ch0 = tex_chars[tile_idx][v][u0];
                ch1 = tex_chars[tile_idx][v][u1];

                c0 = tex_colors[tile_idx][v][u0];
                c1 = tex_colors[tile_idx][v][u1];

                /* Directional wall shading: darken Y-side walls for 3D depth */
                if (side) {
                    c0 = dark_colors[c0];
                    c1 = dark_colors[c1];
                }

                /* Atmospheric distance fog */
                if (is_fog) {
                    ch0 = TEX_DITHER1;
                    ch1 = TEX_DITHER1;
                    c0 = fog_colors[c0];
                    c1 = fog_colors[c1];
                }

                scr[0] = ch0;
                scr[1] = ch1;
                col[0] = c0;
                col[1] = c1;
                scr += 40;
                col += 40;
            }
        }

        /* 3. Fast Perspective Floor (Rows bot to VIEW_HEIGHT-1) */
        for (y = bot; y < VIEW_HEIGHT; ++y) {
            if (y > 21) {
                scr[0] = TEX_FLOOR_LINE;
                scr[1] = TEX_FLOOR_LINE;
                col[0] = C64_BROWN;
                col[1] = C64_BROWN;
            } else if (y > 17) {
                scr[0] = TEX_FLOOR_LINE;
                scr[1] = TEX_FLOOR_LINE;
                col[0] = C64_DARKGRAY;
                col[1] = C64_DARKGRAY;
            } else {
                scr[0] = TEX_DOT;
                scr[1] = TEX_DOT;
                col[0] = C64_BLACK;
                col[1] = C64_BLACK;
            }
            scr += 40;
            col += 40;
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
                /* Empty floor tile */
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
