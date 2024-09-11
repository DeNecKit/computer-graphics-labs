#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "stb_image_write.h"

#define PIXEL(x, y, w, h) ((h)-(int)(y)-1)*(w)*4 + (int)(x)*4

void circle(float r)
{
    if (r <= 0) {
        fprintf(stderr, "Radius must be positive\n");
        exit(1);
    }
    
    const char *filename = "circle.png";
    const int w = r * 2 + fmaxf((int)(r * 0.1f) / 2 * 2, 4) + 1;
    const int h = w;
    const int x1 = w / 2;
    const int y1 = h / 2;
    const int size = w * h * 4;

    unsigned char *data = malloc(size);
    memset(data, 0xff, size);

    int x = 0;
    int y = r;
    int delta = 1 - 2 * r;
    int error = 0;

#define PUTPIXEL(x, y) memset(data + PIXEL((x), (y), w, h), 0, 3)
    
    while (y >= x) {
        PUTPIXEL(x1 + x, y1 + y);
        PUTPIXEL(x1 + x, y1 - y);
        PUTPIXEL(x1 - x, y1 + y);
        PUTPIXEL(x1 - x, y1 - y);
        PUTPIXEL(x1 + y, y1 + x);
        PUTPIXEL(x1 + y, y1 - x);
        PUTPIXEL(x1 - y, y1 + x);
        PUTPIXEL(x1 - y, y1 - x);
        
        error = 2 * (delta + y) - 1;
        if ((delta < 0) && (error <= 0)) {
            delta += 2 * ++x + 1;
            continue;
        }
        if ((delta > 0) && (error > 0)) {
            delta -= 2 * --y + 1;
            continue;
        }
        delta += 2 * (++x - --y);
    }
    
    stbi_write_png(filename, w, h, 4, data, w*4);
    free(data);

    printf("Saved to %s\n", filename);
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Expected radius\n");
        exit(1);
    }

    circle(atof(argv[1]));

    return 0;
}
