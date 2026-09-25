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
        unsigned char half_h = h >> 1;
        signed char top = HALF_VIEW_HEIGHT - half_h;
        signed char bot = HALF_VIEW_HEIGHT + half_h + (h & 1);
        unsigned char col;
        unsigned char ch;
        signed char y;

        if (top < 0) top = 0;
        if (bot > VIEW_HEIGHT) bot = VIEW_HEIGHT;

        /* Wall color & shading */
        switch (tile) {
            case TILE_BLUE_STONE:
                col = (side == 0) ? C64_LIGHTBLUE : C64_BLUE;
                break;
            case TILE_WOOD:
                col = (side == 0) ? C64_ORANGE : C64_BROWN;
                break;
            case TILE_RED_BRICK:
                col = (side == 0) ? C64_LIGHTRED : C64_RED;
                break;
            case TILE_DOOR:
                col = (side == 0) ? C64_YELLOW : C64_ORANGE;
                break;
            case TILE_GREY_STONE:
            default:
                col = (side == 0) ? C64_LIGHTGRAY : C64_DARKGRAY;
                break;
        }

        /* Distance dithering for distant walls */
        if (dist > 75) {
            ch = CH_DITHER1;
        } else {
            ch = CH_SOLID;
        }

        /* 1. Ceiling */
        for (y = 0; y < top; ++y) {
            screen_rows[y][x] = CH_SPACE;
            color_rows[y][x] = C64_BLACK;
        }

        /* 2. Wall */
        for (y = top; y < bot; ++y) {
            screen_rows[y][x] = ch;
            color_rows[y][x] = col;
        }

        /* 3. Floor */
        for (y = bot; y < VIEW_HEIGHT; ++y) {
            screen_rows[y][x] = CH_DOT;
            color_rows[y][x] = C64_DARKGRAY;
        }
    }
}

void render_hud(unsigned int score, unsigned char health, unsigned char fps) {
    char buf[40];
    unsigned char x;
    signed char rx, ry;
    signed char px = (signed char)(player_x >> FP_SHIFT);
    signed char py = (signed char)(player_y >> FP_SHIFT);

    /* Separator line on row 19 */
    for (x = 0; x < SCREEN_COLS; ++x) {
        screen_rows[19][x] = 0x40; /* Horizontal line */
        color_rows[19][x] = C64_LIGHTGRAY;
    }

    /* Row 20: Floor & Score */
    print_at(1, 20, "FLOOR: 1", C64_YELLOW);
    print_at(12, 20, "SCORE:", C64_CYAN);
    buf[0] = '0' + ((score / 1000) % 10);
    buf[1] = '0' + ((score / 100) % 10);
    buf[2] = '0' + ((score / 10) % 10);
    buf[3] = '0' + (score % 10);
    buf[4] = '\0';
    print_at(19, 20, buf, C64_WHITE);

    /* Row 21: Health & Keys */
    print_at(1, 21, "HEALTH:", C64_LIGHTRED);
    buf[0] = '0' + ((health / 100) % 10);
    buf[1] = '0' + ((health / 10) % 10);
    buf[2] = '0' + (health % 10);
    buf[3] = '%';
    buf[4] = '\0';
    print_at(9, 21, buf, C64_GREEN);
    print_at(16, 21, "KEY:[GOLD]", C64_YELLOW);

    /* Row 22: Position & Angle */
    print_at(1, 22, "POS:", C64_LIGHTBLUE);
    buf[0] = '0' + ((px / 10) % 10);
    buf[1] = '0' + (px % 10);
    buf[2] = ',';
    buf[3] = '0' + ((py / 10) % 10);
    buf[4] = '0' + (py % 10);
    buf[5] = '\0';
    print_at(6, 22, buf, C64_WHITE);

    print_at(13, 22, "ANG:", C64_LIGHTBLUE);
    buf[0] = '0' + ((player_angle / 100) % 10);
    buf[1] = '0' + ((player_angle / 10) % 10);
    buf[2] = '0' + (player_angle % 10);
    buf[3] = '\0';
    print_at(18, 22, buf, C64_WHITE);

    /* Row 23: FPS & Controls hint */
    print_at(1, 23, "FPS:", C64_ORANGE);
    buf[0] = '0' + ((fps / 10) % 10);
    buf[1] = '0' + (fps % 10);
    buf[2] = '\0';
    print_at(6, 23, buf, C64_WHITE);
    print_at(10, 23, "[W/A/S/D] MOVE", C64_GRAY);

    /* Row 24: Extra controls */
    print_at(1, 24, "[M] MAP  [Q/E] STRAFE", C64_DARKGRAY);

    /* Mini-Radar in rows 20..24, cols 31..38 */
    print_at(31, 19, "+RADAR+", C64_YELLOW);
    for (ry = -2; ry <= 2; ++ry) {
        unsigned char hud_y = (unsigned char)(22 + ry);
        for (rx = -3; rx <= 4; ++rx) {
            unsigned char hud_x = (unsigned char)(34 + rx);
            signed char mx = px + rx;
            signed char my = py + ry;

            if (rx == 0 && ry == 0) {
                /* Player position marker */
                screen_rows[hud_y][hud_x] = 0x51; /* Ball/Circle */
                color_rows[hud_y][hud_x] = C64_WHITE;
            } else if (mx >= 0 && mx < MAP_WIDTH && my >= 0 && my < MAP_HEIGHT) {
                unsigned char t = game_map[(unsigned char)my][(unsigned char)mx];
                if (t != TILE_EMPTY) {
                    screen_rows[hud_y][hud_x] = CH_SOLID;
                    color_rows[hud_y][hud_x] = (t == TILE_DOOR) ? C64_YELLOW : C64_LIGHTBLUE;
                } else {
                    screen_rows[hud_y][hud_x] = CH_DOT;
                    color_rows[hud_y][hud_x] = C64_DARKGRAY;
                }
            } else {
                screen_rows[hud_y][hud_x] = CH_SPACE;
                color_rows[hud_y][hud_x] = C64_BLACK;
            }
        }
    }
}

void render_fullscreen_map(void) {
    unsigned char x, y;
    signed char px = (signed char)(player_x >> FP_SHIFT);
    signed char py = (signed char)(player_y >> FP_SHIFT);

    /* Clear screen */
    for (y = 0; y < SCREEN_ROWS; ++y) {
        for (x = 0; x < SCREEN_COLS; ++x) {
            screen_rows[y][x] = CH_SPACE;
            color_rows[y][x] = C64_BLACK;
        }
    }

    print_at(10, 1, "=== TACTICAL MAP ===", C64_YELLOW);
    print_at(6, 2, "PLAYER: (O)  WALL: (#)  DOOR: (=)", C64_CYAN);

    /* Draw 16x16 map centered: cols 12..27, rows 4..19 */
    for (y = 0; y < MAP_HEIGHT; ++y) {
        for (x = 0; x < MAP_WIDTH; ++x) {
            unsigned char sx = 12 + x;
            unsigned char sy = 4 + y;
            unsigned char t = game_map[y][x];

            if ((signed char)x == px && (signed char)y == py) {
                screen_rows[sy][sx] = 0x51; /* Player marker */
                color_rows[sy][sx] = C64_WHITE;
            } else if (t == TILE_DOOR) {
                screen_rows[sy][sx] = '=';
                color_rows[sy][sx] = C64_YELLOW;
            } else if (t != TILE_EMPTY) {
                screen_rows[sy][sx] = CH_SOLID;
                color_rows[sy][sx] = (t == TILE_BLUE_STONE) ? C64_LIGHTBLUE :
                                     (t == TILE_RED_BRICK)  ? C64_LIGHTRED :
                                     (t == TILE_WOOD)       ? C64_ORANGE : C64_LIGHTGRAY;
            } else {
                screen_rows[sy][sx] = CH_DOT;
                color_rows[sy][sx] = C64_DARKGRAY;
            }
        }
    }

    print_at(9, 22, "PRESS [M] TO RETURN", C64_GREEN);
}
