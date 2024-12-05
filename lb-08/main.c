#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "stb_image_write.h"

#define error(...)                    \
    do {                              \
        fprintf(stderr, __VA_ARGS__); \
        exit(1);                      \
    } while (0)

#define ARR_LEN(xs) ((int)(sizeof(xs) / sizeof(*(xs))))

typedef uint8_t  u8;
typedef uint32_t u32;

typedef struct { int x, y; } v2;

void line(void *img,
          int x1, int y1, int x2, int y2,
          int w, int h, int comp, u32 clr);

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
    int res = 1;
    if (strcmp(ext, ".jpg") == 0) {
        res = stbi_write_jpg(filename, w, h, comp, data, 100);
    } else if (strcmp(ext, ".png") == 0) {
        res = stbi_write_png(filename, w, h, comp, data, w * comp);
    } else {
        error("Unsupported file extension: \"%s\"\n", ext);
    }
    if (res) {
        printf("Wrote to \"%s\"\n", filename);
    } else {
        error("Failed to write to \"%s\"\n", filename);
    }
}

#define DRAW_AT(img, x, y, clr) ((u32*)(img))[(y)*(w)]

u32 *image_at(u8 *image, v2 point, int w)
{
    return &((u32*)image)[point.y*w + point.x];
}

void draw_at(u8 *image, v2 point, int w, u32 clr)
{
    *image_at(image, point, w) = clr;
}

#define POINTS_STACK_LEN (1 << 16)
v2 points_stack[POINTS_STACK_LEN] = { 0 };
int points_stack_i = 0;

void push_point(v2 p)
{
    assert(points_stack_i < POINTS_STACK_LEN);
    points_stack[points_stack_i++] = p;
}

v2 pop_point()
{
    assert(points_stack_i > 0);
    return points_stack[--points_stack_i];
}

int main(int argc, char **argv)
{
    if (argc < 2) error("Expected output image filename\n");
    const char *filename = argv[1];

    const int w = 500;
    const int h = 500;
    const int comp = 4;
    const u32 black = 0xff000000;
    u8 *image = malloc(w * h * comp);
    memset(image, 0xff, w * h * comp);

    v2 shape[] = {
        {  200 , 150  },
        { -50  , 100  },
        {  200 , 400  },
        {  450 , 400  },
        {  600 , 250  },
        {  300 , 300  }
    };
    int shape_len = ARR_LEN(shape);

    for (int i = 0; i < shape_len; i++) { 
        v2 p1 = shape[i];
        v2 p2 = shape[(i + 1) % shape_len];
        line(image, p1.x, p1.y, p2.x, p2.y, w, h, comp, black);
    }

    bool found = false;
    v2 start_point = { -1, -1 };
    for (int y = 0; y < h; y++) {
        int dots = 0;
        for (int x = 0; x < w; x++) {
            u32 clr = *image_at(image, (v2){ x, y }, w);
            bool is_space = true;
            if (clr == black) {
                if (is_space) {
                    dots++;
                    is_space = false;
                }
                if (dots == 2) {
                    start_point.x = x - 2;
                    start_point.y = y;
                    found = true;
                } else if (dots > 2) {
                    found = false;
                    break;
                }
            } else is_space = true;
        }
        if (found) break;
    }

    push_point(start_point);
    while (points_stack_i > 0) {
        v2 p = pop_point();
        if (*image_at(image, p, w) != black) {
            draw_at(image, p, w, black);
        }
        v2 p1 = { p.x - 1, p.y };
        if (p1.x >= 0 && *image_at(image, p1, w) != black) push_point(p1);
        v2 p2 = { p.x, p.y - 1 };
        if (p2.y >= 0 && *image_at(image, p2, w) != black) push_point(p2);
        v2 p3 = { p.x + 1, p.y };
        if (p3.x < w && *image_at(image, p3, w) != black) push_point(p3);
        v2 p4 = { p.x, p.y + 1 };
        if (p4.y < h && *image_at(image, p4, w) != black) push_point(p4);
    }

    write_image(filename, w, h, comp, image);
}
