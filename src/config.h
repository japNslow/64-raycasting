#ifndef CONFIG_H
#define CONFIG_H

/* 160x200 Multicolor Bitmap Mode (Wolfenstein 3D / DOOM style) */
#define BITMAP_WIDTH        160 /* 160 multicolor pixels wide */
#define BITMAP_HEIGHT       200 /* 200 scanlines high */
#define HALF_BITMAP_HEIGHT  100 /* Center horizon at scanline 100 */
#define SCREEN_COLS         40  /* 40 cells horizontally (4 pixels each = 160) */
#define SCREEN_ROWS         25  /* 25 cells vertically (8 scanlines each = 200) */
#define NUM_RAYS            40  /* 40 rays across 160 pixels */

/* Raycasting settings */
#define FOV_ANGLES          42  /* ~60 degrees in 256-angle circle */
#define MAX_RAY_STEPS       16  /* 16 DDA steps for 16x16 map */

/* Memory locations for VIC-II Bank 1 ($4000-$7FFF) */
#define BITMAP_RAM          ((unsigned char*)0x6000)
#define SCREEN_RAM          ((unsigned char*)0x4000)
#define COLOR_RAM           ((unsigned char*)0xD800)
#define CIA1_PRA            (*(volatile unsigned char*)0xDC00)
#define CIA1_PRB            (*(volatile unsigned char*)0xDC01)
#define CIA2_PRA            (*(volatile unsigned char*)0xDD00)
#define JOYSTICK_PORT2      (*(volatile unsigned char*)0xDC00)

/* C64 VIC-II Color codes */
#define C64_BLACK           0
#define C64_WHITE           1
#define C64_RED             2
#define C64_CYAN            3
#define C64_PURPLE          4
#define C64_GREEN           5
#define C64_BLUE            6
#define C64_YELLOW          7
#define C64_ORANGE          8
#define C64_BROWN           9
#define C64_LIGHTRED        10
#define C64_DARKGRAY        11
#define C64_GRAY            12
#define C64_LIGHTGREEN      13
#define C64_LIGHTBLUE       14
#define C64_LIGHTGRAY       15

#endif /* CONFIG_H */
