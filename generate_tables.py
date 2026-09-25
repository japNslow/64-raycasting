import math

with open('src/tables.c', 'w', encoding='utf-8') as f:
    f.write('/* Generated trigonometric and projection lookup tables */\n')
    f.write('#include "tables.h"\n\n')

    # Sin table
    f.write('const signed int sin_table[256] = {\n')
    for i in range(256):
        angle = 2.0 * math.pi * i / 256.0
        val = int(round(math.sin(angle) * 256.0))
        if i % 8 == 0:
            f.write('    ')
        f.write(f'{val:6d},')
        if i % 8 == 7:
            f.write('\n')
    f.write('};\n\n')

    # Delta table (delta_x = 256 / |cos(a)|)
    f.write('const unsigned int delta_x_table[256] = {\n')
    for i in range(256):
        angle = 2.0 * math.pi * i / 256.0
        ca = abs(math.cos(angle))
        if ca < 0.001:
            d = 32767
        else:
            d = int(round(256.0 / ca))
            if d > 32767:
                d = 32767
        if i % 8 == 0:
            f.write('    ')
        f.write(f'{d:6d},')
        if i % 8 == 7:
            f.write('\n')
    f.write('};\n\n')

    # Ray angle offset table
    f.write('const signed char ray_angle_offset[40] = {\n')
    fov_deg = 60.0
    ray_offsets = []
    cos_rays = []
    for col in range(40):
        camera_x = 2.0 * col / 39.0 - 1.0
        ray_ang_rad = math.atan(camera_x * math.tan(math.radians(fov_deg / 2.0)))
        ray_ang_256 = int(round(ray_ang_rad * 256.0 / (2.0 * math.pi)))
        ray_offsets.append(ray_ang_256)
        cos_rays.append(int(round(math.cos(ray_ang_rad) * 256.0)))
        if col % 8 == 0:
            f.write('    ')
        f.write(f'{ray_ang_256:4d},')
        if col % 8 == 7:
            f.write('\n')
    if len(ray_offsets) % 8 != 0:
        f.write('\n')
    f.write('};\n\n')

    # Cos ray table for fish-eye correction
    f.write('const unsigned char cos_ray_table[40] = {\n')
    for col in range(40):
        cr = min(255, cos_rays[col])
        if col % 8 == 0:
            f.write('    ')
        f.write(f'{cr:4d},')
        if col % 8 == 7:
            f.write('\n')
    if len(cos_rays) % 8 != 0:
        f.write('\n')
    f.write('};\n\n')

    # Height table
    f.write('const unsigned char height_table[256] = {\n')
    for dist in range(256):
        if dist < 6:
            h = 19
        else:
            h = int(round(360.0 / float(dist)))
            if h > 19:
                h = 19
            if h < 1:
                h = 1
        if dist % 8 == 0:
            f.write('    ')
        f.write(f'{h:4d},')
        if dist % 8 == 7:
            f.write('\n')
    f.write('};\n')

print('Generated src/tables.c successfully')
