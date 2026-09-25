#include <conio.h>
#include "config.h"
#include "fixed.h"
#include "tables.h"
#include "map.h"
#include "raycast.h"
#include "render.h"

#define MOVE_SPEED  50   /* Smooth movement speed in 8.8 fixed units */
#define TURN_SPEED  7    /* Smooth turning speed (~10 deg per frame) */

/* SID chip registers for sound effects */
#define SID_V1_FREQ_LO  (*(volatile unsigned char*)0xD400)
#define SID_V1_FREQ_HI  (*(volatile unsigned char*)0xD401)
#define SID_V1_CTRL     (*(volatile unsigned char*)0xD404)
#define SID_V1_AD       (*(volatile unsigned char*)0xD405)
#define SID_V1_SR       (*(volatile unsigned char*)0xD406)
#define SID_VOL         (*(volatile unsigned char*)0xD418)

static unsigned char joy_up = 0;
static unsigned char joy_down = 0;
static unsigned char joy_left = 0;
static unsigned char joy_right = 0;
static unsigned char joy_fire = 0;
static unsigned char key_map = 0;
static unsigned char key_exit = 0;

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

    /* Wall sliding collision detection */
    if (tx < MAP_WIDTH && game_map[cur_y][tx] == TILE_EMPTY) {
        player_x = new_x;
    }
    if (ty < MAP_HEIGHT && game_map[ty][cur_x] == TILE_EMPTY) {
        player_y = new_y;
    }
}

/* Read Joystick 2 (CIA1 Port A $DC00) and keyboard */
static void read_controls(void) {
    unsigned char joy;

    joy_up = joy_down = joy_left = joy_right = joy_fire = key_map = key_exit = 0;

    /* 1. Read Commodore 64 Joystick in Port 2 ($DC00) */
    joy = JOYSTICK_PORT2;
    if (!(joy & 0x01)) joy_up = 1;     /* Up */
    if (!(joy & 0x02)) joy_down = 1;   /* Down */
    if (!(joy & 0x04)) joy_left = 1;   /* Left */
    if (!(joy & 0x08)) joy_right = 1;  /* Right */
    if (!(joy & 0x10)) joy_fire = 1;   /* Fire button */

    /* 2. Check Keyboard input (WASD / Arrows / M / X) */
    while (kbhit()) {
        char ch = cgetc();
        if (ch == 'w' || ch == 'W' || ch == 145) joy_up = 1;
        else if (ch == 's' || ch == 'S' || ch == 17) joy_down = 1;
        else if (ch == 'a' || ch == 'A' || ch == 157) joy_left = 1;
        else if (ch == 'd' || ch == 'D' || ch == 29) joy_right = 1;
        else if (ch == ' ' || ch == 13) joy_fire = 1;
        else if (ch == 'm' || ch == 'M') key_map = 1;
        else if (ch == 'x' || ch == 'X') key_exit = 1;
    }
}

int main(void) {
    unsigned char map_mode = 0;
    unsigned char last_map_key = 0;
    unsigned char last_fire = 0;
    unsigned char running = 1;

    init_renderer();
    init_raycaster();

    while (kbhit()) {
        cgetc();
    }

    while (running) {
        if (map_mode) {
            render_fullscreen_map();
        } else {
            raycast_all();
            render_frame();
        }

        /* Read controls from Joystick Port 2 and keyboard */
        read_controls();

        if (key_exit) {
            running = 0;
            break;
        }

        /* Toggle 3D Map mode */
        if (key_map && !last_map_key) {
            map_mode = !map_mode;
        }
        last_map_key = key_map;

        /* If Fire button is held while turning: Strafe! */
        if (joy_fire && joy_left) {
            unsigned char strafe_ang = (unsigned char)(player_angle - 64);
            fixed dx = (fixed)(((signed long)COS_LOOKUP(strafe_ang) * MOVE_SPEED) >> FP_SHIFT);
            fixed dy = (fixed)(((signed long)SIN_LOOKUP(strafe_ang) * MOVE_SPEED) >> FP_SHIFT);
            try_move(dx, dy);
        } else if (joy_fire && joy_right) {
            unsigned char strafe_ang = (unsigned char)(player_angle + 64);
            fixed dx = (fixed)(((signed long)COS_LOOKUP(strafe_ang) * MOVE_SPEED) >> FP_SHIFT);
            fixed dy = (fixed)(((signed long)SIN_LOOKUP(strafe_ang) * MOVE_SPEED) >> FP_SHIFT);
            try_move(dx, dy);
        } else {
            /* Normal rotation */
            if (joy_left) {
                player_angle = (unsigned char)(player_angle - TURN_SPEED);
            }
            if (joy_right) {
                player_angle = (unsigned char)(player_angle + TURN_SPEED);
            }
        }

        /* Forward / Backward movement */
        if (joy_up) {
            fixed dx = (fixed)(((signed long)COS_LOOKUP(player_angle) * MOVE_SPEED) >> FP_SHIFT);
            fixed dy = (fixed)(((signed long)SIN_LOOKUP(player_angle) * MOVE_SPEED) >> FP_SHIFT);
            try_move(dx, dy);
            play_step_sound();
        }
        if (joy_down) {
            fixed dx = (fixed)(((signed long)COS_LOOKUP(player_angle) * MOVE_SPEED) >> FP_SHIFT);
            fixed dy = (fixed)(((signed long)SIN_LOOKUP(player_angle) * MOVE_SPEED) >> FP_SHIFT);
            try_move(-dx, -dy);
            play_step_sound();
        }

        /* Fire button (tap): Open door in front of player */
        if (joy_fire && !last_fire && !joy_left && !joy_right) {
            unsigned char front_ang = player_angle;
            fixed fx = player_x + (fixed)(((signed long)COS_LOOKUP(front_ang) * 300) >> FP_SHIFT);
            fixed fy = player_y + (fixed)(((signed long)SIN_LOOKUP(front_ang) * 300) >> FP_SHIFT);
            unsigned char ftx = (unsigned char)(fx >> FP_SHIFT);
            unsigned char fty = (unsigned char)(fy >> FP_SHIFT);

            if (ftx < MAP_WIDTH && fty < MAP_HEIGHT) {
                if (game_map[fty][ftx] == TILE_DOOR) {
                    game_map[fty][ftx] = TILE_EMPTY;
                    play_step_sound();
                }
            }
        }
        last_fire = joy_fire;
    }

    return 0;
}
