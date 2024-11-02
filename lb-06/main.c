#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "stb_image_write.h"

typedef uint8_t  u8;
typedef uint32_t u32;

#define error(...)                    \
    do {                              \
        fprintf(stderr, __VA_ARGS__); \
        exit(1);                      \
    } while (0)

const char *get_file_ext(const char *filename)
{
    while (*filename && *filename != '.') filename++;
    return filename;
}

void write_image(const char *filename,
                 int w, int h, int comp,
                 unsigned char *data)
{
    const char *ext = get_file_ext(filename);
    if (strcmp(ext, ".jpg") == 0) {
        stbi_write_jpg(filename, w, h, comp, data, 100);
    } else if (strcmp(ext, ".png") == 0) {
        stbi_write_png(filename, w, h, comp, data, w * comp);
    } else {
        fprintf(stderr, "Unsupported file extension: \"%s\"\n", ext);
        exit(1);
    }
    printf("Wrote to \"%s\"\n", filename);
}

int parse_int(const char *msg)
{
    printf("%s", msg);
    int res;
    if (scanf("%d", &res) != 1) {
        error("Failed to parse integer\n");
    }
    return res;
}

#define IMG_COMP 4
u8 *img = NULL;
int wx1, wy1, wx2, wy2;
const u32 clr = 0xff000000;
const int eps = 1;

void line(void *img,
          int x1, int y1, int x2, int y2,
          int w, int h, u32 clr);

u8 get_code(int x, int y)
{
    u8 res = 0;
    res |= x < wx1;
    res |= (y < wy1) << 1;
    res |= (y > wy2) << 2;
    res |= (x > wx2) << 3;
    return res;
}

bool at_border(int x, int y, u8 code)
{
    return ((x == wx1 + eps || x == wx1 - eps) && code & 0b0001) ||
           ((y == wy1 + eps || y == wy1 - eps) && code & 0b0010 >> 1) ||
           ((y == wy2 + eps || y == wy2 - eps) && code & 0b0100 >> 2) ||
           ((x == wx2 + eps || x == wx2 - eps) && code & 0b1000 >> 3);
}

void move(int x1, int y1, int x2, int y2,
          int *x, int *y)
{
    assert(x != NULL);
    assert(y != NULL);

    if (!get_code(x2, y2)) {
        *x = x2;
        *y = y2;
        return;
    }

    while (true) {
        int cx = (x1 + x2) / 2;
        int cy = (y1 + y2) / 2;
        u8 cc = get_code(cx, cy);
        if (at_border(cx, cy, get_code(x2, y2))) {
            *x = cx;
            *y = cy;
            return;
        }
        if (!cc) {
            x1 = cx;
            y1 = cy;
        } else {
            x2 = cx;
            y2 = cy;
        }
        if (abs(x1 - x2) <= eps && abs(y1 - y2) <= eps) {
            *x = x1;
            *y = y1;
            return;
        }
    }
}

int main(void)
{
    wx1 = parse_int("Window x = ");
    wy1 = parse_int("Window y = ");
    int w, h;
    w = parse_int("Window width = ");
    if (w <= 0) error("Window width must be positive\n");
    h = parse_int("Window height = ");
    if (h <= 0) error("Window height must be positive\n");

    wx2 = wx1 + w - 1;
    wy2 = wy1 + h - 1;

    img = malloc(w * h * IMG_COMP);
    if (img == NULL) {
        error("Failed to allocate memory\n");
    }
    memset(img, 0xff, w * h * IMG_COMP);

    int n = parse_int("Number of lines = ");
    for (int i = 0; i < n; i++) {
        printf("Line #%d\n", i + 1);
        int x1 = parse_int("  x1 = ");
        int y1 = parse_int("  y1 = ");
        int x2 = parse_int("  x2 = ");
        int y2 = parse_int("  y2 = ");

        u8 c1 = get_code(x1, y1);
        u8 c2 = get_code(x2, y2);

        if (c1 & c2) continue;

        if (!c1 && !c2) {
            x1 -= wx1; x2 -= wx1;
            y1 -= wy1; y2 -= wx1;
            line(img, x1, y1, x2, y2, w, h, clr);
            continue;
        }

        int cx = (x1 + x2) / 2;
        int cy = (y1 + y2) / 2;
        u8 cc = get_code(cx, cy);

        while (true) {
            if ((x1 == cx && y1 == cy) ||
                (x2 == cx && y2 == cy)) break;
            if (cc & c1) {
                x1 = cx;
                y1 = cy;
                cx = (x1 + x2) / 2;
                cy = (y1 + y2) / 2;
                cc = get_code(cx, cy);
            } else if (cc & c2) {
                x2 = cx;
                y2 = cy;
                cx = (x1 + x2) / 2;
                cy = (y1 + y2) / 2;
                cc = get_code(cx, cy);
            } else {
                move(cx, cy, x1, y1, &x1, &y1);
                move(cx, cy, x2, y2, &x2, &y2);
                x1 -= wx1; x2 -= wx1;
                y1 -= wy1; y2 -= wx1;
                line(img, x1, y1, x2, y2, w, h, clr);
                break;
            }
        }
    }

    printf("Output filename = ");
    char output[1024] = { 0 };
    if (scanf("%1023s", output) != 1) {
        error("Failed to parse output filename\n");
    }

    write_image(output, w, h, IMG_COMP, img);

    free(img);
}
