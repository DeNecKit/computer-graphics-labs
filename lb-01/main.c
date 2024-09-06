#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>
#include "stb_image_write.h"

#define PIXEL(x, y, w, h) ((h)-(int)(y)-1)*(w)*4 + (int)(x)*4

float signf(float x) { return x > 0 ? 1.f : x < 0 ? -1.f : 0.f; }

void line_cda(float x1, float y1, float x2, float y2)
{
    const char *filename = "cda.png";
    const float mx = fminf(x1, x2);
    const float my = fminf(y1, y2);
    x1 += -mx + 2;
    x2 += -mx + 2;
    y1 += -my + 2;
    y2 += -my + 2;

    float dx = fabsf(x2 - x1);
    float dy = fabsf(y2 - y1);

    const int w = (int)fmaxf(dx, 1.f) + 4 + (x1 == x2 ? 0 : 1);
    const int h = (int)fmaxf(dy, 1.f) + 4 + (y1 == y2 ? 0 : 1);
    const int size = w*h*4;
    unsigned char *data = malloc(size);
    memset(data, 0xff, size);

    if (x1 == x2 && y1 == y2) {
        memset(data + PIXEL(x1, y1, w, h), 0, 3);
        stbi_write_png(filename, w, h, 4, data, w*4);
        free(data);
        return;
    }

    float l = fmaxf(dx, dy);
    dx = (x2 - x1) / l;
    dy = (y2 - y1) / l;

    float x = x1;
    float y = y1;

    for (int i = 0; i < l; i++) {
        memset(data + PIXEL(x, y, w, h), 0, 3);
        x += dx;
        y += dy;
    }
    memset(data + PIXEL(x2, y2, w, h), 0, 3);

    stbi_write_png(filename, w, h, 4, data, w*4);
    free(data);
}

void line_bresenham(float x1, float y1, float x2, float y2)
{
    const char *filename = "bresenham.png";
    const float mx = fminf(x1, x2);
    const float my = fminf(y1, y2);
    x1 += -mx + 2;
    x2 += -mx + 2;
    y1 += -my + 2;
    y2 += -my + 2;

    float dx = x2 - x1;
    float dy = y2 - y1;
    float sx = signf(dx);
    float sy = signf(dy);
    dx = fabsf(dx);
    dy = fabsf(dy);

    const int w = (int)fmaxf(dx, 1.f) + 4 + (x1 == x2 ? 0 : 1);
    const int h = (int)fmaxf(dy, 1.f) + 4 + (y1 == y2 ? 0 : 1);
    const int size = w*h*4;
    unsigned char *data = malloc(size);
    memset(data, 0xff, size);

    if (x1 == x2 && y1 == y2) {
        memset(data + PIXEL(x1, y1, w, h), 0, 3);
        stbi_write_png(filename, w, h, 4, data, w*4);
        free(data);
        return;
    }

    bool flag = false;
    if (dy > dx) {
        float tmp = dx;
        dx = dy;
        dy = tmp;
        flag = true;
    }

    const float df = dy / dx;
    float f = df - 0.5f;
    float x = x1;
    float y = y1;

    while (true) {
        if (sx > 0 && x > x2 ||
            sx < 0 && x < x2 ||
            sy > 0 && y > y2 ||
            sy < 0 && y < y2) break;
        memset(data + PIXEL(x, y, w, h), 0, 3);
        if (f >= 0) {
            if (flag) x += sx;
            else      y += sy;
            f -= 1.f;
        }
        if (f < 0) {
            if (flag) y += sy;
            else      x += sx;
            f += df;
        }
    }

    stbi_write_png(filename, w, h, 4, data, w*4);
    free(data);
}

void line_bresenham_int(float x1, float y1, float x2, float y2)
{
    const char *filename = "bresenham_int.png";
    const float mx = fminf(x1, x2);
    const float my = fminf(y1, y2);
    x1 += -mx + 2;
    x2 += -mx + 2;
    y1 += -my + 2;
    y2 += -my + 2;

    float dx = x2 - x1;
    float dy = y2 - y1;
    float sx = signf(dx);
    float sy = signf(dy);
    dx = fabsf(dx);
    dy = fabsf(dy);

    const int w = (int)fmaxf(dx, 1.f) + 4 + (x1 == x2 ? 0 : 1);
    const int h = (int)fmaxf(dy, 1.f) + 4 + (y1 == y2 ? 0 : 1);
    const int size = w*h*4;
    unsigned char *data = malloc(size);
    memset(data, 0xff, size);

    if (x1 == x2 && y1 == y2) {
        memset(data + PIXEL(x1, y1, w, h), 0, 3);
        stbi_write_png(filename, w, h, 4, data, w*4);
        free(data);
        return;
    }

    bool flag = false;
    if (dy > dx) {
        int tmp = dx;
        dx = dy;
        dy = tmp;
        flag = true;
    }

    int f = 2*dy - dx;
    int x = x1;
    int y = y1;

    while (true) {
        if (sx > 0 && x > x2 ||
            sx < 0 && x < x2 ||
            sy > 0 && y > y2 ||
            sy < 0 && y < y2) break;
        memset(data + PIXEL(x, y, w, h), 0, 3);
        if (f >= 0) {
            if (flag) x += sx;
            else      y += sy;
            f -= 2*dx;
        }
        if (f < 0) {
            if (flag) y += sy;
            else      x += sx;
            f += 2*dy;
        }
    }

    stbi_write_png(filename, w, h, 4, data, w*4);
    free(data);
}

int main(int argc, char *argv[])
{
    if (argc != 5) {
        fprintf(stderr, "Expected coordinates\n");
        exit(1);
    }

    float coords[4] = { 0.f };
    for (int i = 1; i < 5; i++) {
        coords[i - 1] = atof(argv[i]);
    }

    line_cda(coords[0], coords[1], coords[2], coords[3]);
    line_bresenham(coords[0], coords[1], coords[2], coords[3]);
    line_bresenham_int(coords[0], coords[1], coords[2], coords[3]);

    return 0;
}
