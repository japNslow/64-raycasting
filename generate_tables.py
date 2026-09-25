import math

# C64 Colors
C64_BLACK       = 0
C64_WHITE       = 1
C64_RED         = 2
C64_CYAN        = 3
C64_PURPLE      = 4
C64_GREEN       = 5
C64_BLUE        = 6
C64_YELLOW      = 7
C64_ORANGE      = 8
C64_BROWN       = 9
C64_LIGHTRED    = 10
C64_DARKGRAY    = 11
C64_GRAY        = 12
C64_LIGHTGREEN  = 13
C64_LIGHTBLUE   = 14
C64_LIGHTGRAY   = 15

with open('src/tables.c', 'w', encoding='utf-8') as f:
    f.write('/* Generated LUT tables for 160x200 Multicolor Bitmap Raycasting */\n')
    f.write('#include "tables.h"\n\n')

    # 1. Sin table
    f.write('const signed int sin_table[256] = {\n')
    for i in range(256):
        angle = 2.0 * math.pi * i / 256.0
        val = int(round(math.sin(angle) * 256.0))
        if i % 8 == 0: f.write('    ')
        f.write(f'{val:6d},')
        if i % 8 == 7: f.write('\n')
    f.write('};\n\n')

    # 2. Delta table (delta_x = 256 / |cos(a)|)
    f.write('const unsigned int delta_x_table[256] = {\n')
    for i in range(256):
        angle = 2.0 * math.pi * i / 256.0
        ca = abs(math.cos(angle))
        if ca < 0.05:
            d = 2048
        else:
            d = int(round(256.0 / ca))
            if d > 2048: d = 2048
        if i % 8 == 0: f.write('    ')
        f.write(f'{d:6d},')
        if i % 8 == 7: f.write('\n')
    f.write('};\n\n')

    # 3. Ray angle offsets for 40 rays (covering 160 horizontal pixels, 1 ray per 4-pixel column)
    fov_deg = 60.0
    ray_offsets = []
    cos_rays = []
    for i in range(40):
        camera_x = 2.0 * i / 39.0 - 1.0
        ray_ang_rad = math.atan(camera_x * math.tan(math.radians(fov_deg / 2.0)))
        ray_ang_256 = int(round(ray_ang_rad * 256.0 / (2.0 * math.pi)))
        ray_offsets.append(ray_ang_256)
        cos_rays.append(int(round(math.cos(ray_ang_rad) * 256.0)))

    f.write('const signed char ray_angle_offset[NUM_RAYS] = {\n    ')
    for i, ro in enumerate(ray_offsets):
        f.write(f'{ro:4d},')
        if i % 8 == 7 and i != 39: f.write('\n    ')
    f.write('\n};\n\n')

    f.write('const unsigned char cos_ray_table[NUM_RAYS] = {\n    ')
    for i, cr in enumerate(cos_rays):
        val = min(255, cr)
        f.write(f'{val:4d},')
        if i % 8 == 7 and i != 39: f.write('\n    ')
    f.write('\n};\n\n')

    # 4. Height table (projected height in scanlines: 0..200)
    f.write('const unsigned char height_table[256] = {\n')
    for dist in range(256):
        if dist < 19:
            h = 200
        else:
            h = int(round(3800.0 / float(dist)))
            if h > 200: h = 200
            if h < 2: h = 2
        if dist % 8 == 0: f.write('    ')
        f.write(f'{h:4d},')
        if dist % 8 == 7: f.write('\n')
    f.write('};\n\n')

    # 5. v_step_table: (16 << 8) / wall_h for wall_h from 0 to 200
    f.write('const unsigned int v_step_table[201] = {\n    ')
    for h in range(201):
        step = 0 if h == 0 else int(round((16 * 256) / h))
        f.write(f'{step:5d},')
        if h % 8 == 7 and h != 200: f.write('\n    ')
    f.write('\n};\n\n')

    # 6. Bitmap textures: 5 tiles x 16 scanlines (each byte = 4 multicolor pixels)
    # Bits: 00=BG (Black), 01=Col1 (Mortar/Shadow), 10=Col2 (Wall), 11=Col3 (Highlight)
    bmp_textures = [
        # Tile 0: DOOM Steel Wall
        [
            0x55, 0xFA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0x0A,
            0x55, 0xAF, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0x55
        ],
        # Tile 1: Blue Stone with Golden Wolfenstein Medallion
        [
            0x55, 0x5A, 0xAA, 0xAA, 0xAA, 0xAF, 0xFF, 0xFF,
            0xFF, 0xFA, 0xAA, 0xAA, 0xA5, 0xAA, 0x55, 0x55
        ],
        # Tile 2: Wood Paneling
        [
            0x55, 0x5A, 0xAA, 0x5A, 0xAA, 0x5A, 0xAA, 0x5A,
            0xAA, 0x5A, 0xAA, 0x5A, 0xAA, 0x5A, 0xAA, 0x55
        ],
        # Tile 3: Red Brick
        [
            0x55, 0x5A, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0x55,
            0xA5, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0x55
        ],
        # Tile 4: DOOM AirLock Door
        [
            0x55, 0xFA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0x5F,
            0xFF, 0x5F, 0xAA, 0xAA, 0xAF, 0xAA, 0xAA, 0x55
        ]
    ]

    f.write('const unsigned char bmp_textures[5][16] = {\n')
    for t in range(5):
        f.write('    { /* Tile ' + str(t+1) + ' */\n        ')
        for v in range(16):
            f.write(f'0x{bmp_textures[t][v]:02X}, ')
            if v == 7: f.write('\n        ')
        f.write('\n    },\n')
    f.write('};\n\n')

    # 7. Palettes per tile
    # Color 1 = Upper nibble of Screen RAM, Color 2 = Lower nibble of Screen RAM
    # Color 3 = Color RAM ($D800)
    tile_color_screen = [
        (C64_DARKGRAY << 4) | C64_LIGHTGRAY, # Tile 0: Steel (Dark Gray + Light Gray)
        (C64_BLUE << 4)     | C64_LIGHTBLUE, # Tile 1: Blue Stone (Blue + Light Blue)
        (C64_BROWN << 4)    | C64_ORANGE,    # Tile 2: Wood (Brown + Orange)
        (C64_WHITE << 4)    | C64_LIGHTRED,  # Tile 3: Red Brick (White mortar + Light Red)
        (C64_DARKGRAY << 4) | C64_LIGHTGRAY  # Tile 4: Door (Dark Gray + Light Gray)
    ]
    tile_color_screen_dark = [
        (C64_BLACK << 4)    | C64_GRAY,      # Tile 0: Shaded Steel
        (C64_BLACK << 4)    | C64_BLUE,      # Tile 1: Shaded Blue
        (C64_BLACK << 4)    | C64_BROWN,     # Tile 2: Shaded Wood
        (C64_LIGHTGRAY << 4)| C64_RED,       # Tile 3: Shaded Brick
        (C64_BLACK << 4)    | C64_DARKGRAY   # Tile 4: Shaded Door
    ]
    tile_color_ram = [
        C64_WHITE,   # Tile 0: White rivets
        C64_YELLOW,  # Tile 1: Gold medallion
        C64_YELLOW,  # Tile 2: Wood highlight
        C64_YELLOW,  # Tile 3: Brick highlight
        C64_YELLOW   # Tile 4: Gold electronic lock
    ]

    f.write('const unsigned char tile_color_screen[5] = {\n    ' + ', '.join(f'0x{c:02X}' for c in tile_color_screen) + '\n};\n\n')
    f.write('const unsigned char tile_color_screen_dark[5] = {\n    ' + ', '.join(f'0x{c:02X}' for c in tile_color_screen_dark) + '\n};\n\n')
    f.write('const unsigned char tile_color_ram[5] = {\n    ' + ', '.join(f'{c:2d}' for c in tile_color_ram) + '\n};\n')

print('Generated 160x200 tables.c successfully')
