#include <conio.h>
#include "config.h"
#include "fixed.h"
#include "tables.h"
#include "map.h"
#include "raycast.h"
#include "render.h"

#define MOVE_SPEED  60   /* In 8.8 fixed units (~0.23 tile per step) */
#define TURN_SPEED  8    /* ~11 degrees per turn */

/* SID chip registers for sound effects */
#define SID_V1_FREQ_LO  (*(volatile unsigned char*)0xD400)
#define SID_V1_FREQ_HI  (*(volatile unsigned char*)0xD401)
#define SID_V1_CTRL     (*(volatile unsigned char*)0xD404)
#define SID_V1_AD       (*(volatile unsigned char*)0xD405)
#define SID_V1_SR       (*(volatile unsigned char*)0xD406)
#define SID_VOL         (*(volatile unsigned char*)0xD418)

static void play_step_sound(void) {
    SID_VOL = 15;
    SID_V1_AD = 0x05;     /* Fast attack, short decay */
    SID_V1_SR = 0x00;     /* Zero sustain */
    SID_V1_FREQ_LO = 0x20;
    SID_V1_FREQ_HI = 0x04; /* Low thump sound */
    SID_V1_CTRL = 0x81;    /* Noise waveform + Gate bit on */
}

static void try_move(fixed dx, fixed dy) {
    fixed new_x = player_x + dx;
    fixed new_y = player_y + dy;
    unsigned char tx = (unsigned char)(new_x >> FP_SHIFT);
    unsigned char ty = (unsigned char)(new_y >> FP_SHIFT);
    unsigned char cur_x = (unsigned char)(player_x >> FP_SHIFT);
    unsigned char cur_y = (unsigned char)(player_y >> FP_SHIFT);

    /* Wall sliding collision detection with margin */
    if (tx < MAP_WIDTH && game_map[cur_y][tx] == TILE_EMPTY) {
        player_x = new_x;
    }
    if (ty < MAP_HEIGHT && game_map[ty][cur_x] == TILE_EMPTY) {
        player_y = new_y;
    }
}

int main(void) {
    unsigned char map_mode = 0;
    unsigned int score = 150;
    unsigned char health = 100;
    unsigned char fps = 18;
    unsigned int frame_counter = 0;
    unsigned char running = 1;

    init_renderer();
    init_raycaster();

    /* Clear keyboard buffer */
    while (kbhit()) {
        cgetc();
    }

    while (running) {
        frame_counter++;

        if (map_mode) {
            render_fullscreen_map();
        } else {
            raycast_all();
            render_frame();
            render_hud(score, health, fps);
        }

        /* Check keyboard input */
        if (kbhit()) {
            char key = cgetc();

            switch (key) {
                case 'x':
                case 'X':
                    running = 0;
                    break;
                case 'w':
                case 'W':
                case 145: /* Up arrow */
                    {
                        fixed dx = (fixed)(((signed long)COS_LOOKUP(player_angle) * MOVE_SPEED) >> FP_SHIFT);
                        fixed dy = (fixed)(((signed long)SIN_LOOKUP(player_angle) * MOVE_SPEED) >> FP_SHIFT);
                        try_move(dx, dy);
                        play_step_sound();
                    }
                    break;

                case 's':
                case 'S':
                case 17: /* Down arrow */
                    {
                        fixed dx = (fixed)(((signed long)COS_LOOKUP(player_angle) * MOVE_SPEED) >> FP_SHIFT);
                        fixed dy = (fixed)(((signed long)SIN_LOOKUP(player_angle) * MOVE_SPEED) >> FP_SHIFT);
                        try_move(-dx, -dy);
                        play_step_sound();
                    }
                    break;

                case 'a':
                case 'A':
                case 157: /* Left arrow */
                    player_angle = (unsigned char)(player_angle - TURN_SPEED);
                    break;

                case 'd':
                case 'D':
                case 29: /* Right arrow */
                    player_angle = (unsigned char)(player_angle + TURN_SPEED);
                    break;

                case 'q':
                case 'Q': /* Strafe left */
                    {
                        unsigned char strafe_ang = (unsigned char)(player_angle - 64);
                        fixed dx = (fixed)(((signed long)COS_LOOKUP(strafe_ang) * MOVE_SPEED) >> FP_SHIFT);
                        fixed dy = (fixed)(((signed long)SIN_LOOKUP(strafe_ang) * MOVE_SPEED) >> FP_SHIFT);
                        try_move(dx, dy);
                    }
                    break;

                case 'e':
                case 'E': /* Strafe right */
                    {
                        unsigned char strafe_ang = (unsigned char)(player_angle + 64);
                        fixed dx = (fixed)(((signed long)COS_LOOKUP(strafe_ang) * MOVE_SPEED) >> FP_SHIFT);
                        fixed dy = (fixed)(((signed long)SIN_LOOKUP(strafe_ang) * MOVE_SPEED) >> FP_SHIFT);
                        try_move(dx, dy);
                    }
                    break;

                case 'm':
                case 'M': /* Toggle fullscreen map */
                    map_mode = !map_mode;
                    break;

                case ' ': /* Action / Open door */
                    {
                        /* Check tile directly in front of player */
                        unsigned char front_ang = player_angle;
                        fixed fx = player_x + (fixed)(((signed long)COS_LOOKUP(front_ang) * 300) >> FP_SHIFT);
                        fixed fy = player_y + (fixed)(((signed long)SIN_LOOKUP(front_ang) * 300) >> FP_SHIFT);
                        unsigned char ftx = (unsigned char)(fx >> FP_SHIFT);
                        unsigned char fty = (unsigned char)(fy >> FP_SHIFT);

                        if (ftx < MAP_WIDTH && fty < MAP_HEIGHT) {
                            if (game_map[fty][ftx] == TILE_DOOR) {
                                /* Open door by setting tile empty */
                                ((unsigned char*)game_map)[fty * MAP_WIDTH + ftx] = TILE_EMPTY;
                                score += 100;
                            }
                        }
                    }
                    break;

                default:
                    break;
            }
        }

        /* Check Commodore Joystick in Port 2 */
        {
            unsigned char joy = JOYSTICK_PORT2;
            if (!(joy & 0x01)) { /* Up */
                fixed dx = (fixed)(((signed long)COS_LOOKUP(player_angle) * MOVE_SPEED) >> FP_SHIFT);
                fixed dy = (fixed)(((signed long)SIN_LOOKUP(player_angle) * MOVE_SPEED) >> FP_SHIFT);
                try_move(dx, dy);
            }
            if (!(joy & 0x02)) { /* Down */
                fixed dx = (fixed)(((signed long)COS_LOOKUP(player_angle) * MOVE_SPEED) >> FP_SHIFT);
                fixed dy = (fixed)(((signed long)SIN_LOOKUP(player_angle) * MOVE_SPEED) >> FP_SHIFT);
                try_move(-dx, -dy);
            }
            if (!(joy & 0x04)) { /* Left */
                player_angle = (unsigned char)(player_angle - TURN_SPEED);
            }
            if (!(joy & 0x08)) { /* Right */
                player_angle = (unsigned char)(player_angle + TURN_SPEED);
            }
        }
    }

    return 0;
}
