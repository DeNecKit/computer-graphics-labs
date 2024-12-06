#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "stb_image.h"
#include "stb_image_write.h"

#define error(...)                    \
    do {                              \
        fprintf(stderr, __VA_ARGS__); \
        exit(1);                      \
    } while (0)

#define PI 3.141592653589793f

#define MIN(a, b) ((a) < (b) ? a : b)
#define MAX(a, b) ((a) > (b) ? a : b)

typedef uint8_t  u8;
typedef uint32_t u32;

typedef struct {
    float *data;
    int rows, cols;
} table_t;

table_t *table_new(int rows, int cols)
{
    table_t *res = malloc(sizeof(*res));
    *res = (table_t) {
        .data = calloc(rows * cols, sizeof(float)),
        .rows = rows, .cols = cols
    };
    return res;
}

float *table_at(const table_t *table, int row, int col)
{
    return &table->data[row * table->cols + col];
}

void table_free(table_t *table)
{
    free(table->data);
    free(table);
}

void print_table(table_t *table)
{
    for (int row = 0; row < table->rows; row++) {
        for (int col = 0; col < table->cols; col++) {
            printf("%.2f\t", table->data[row * table->cols + col]);
        }
        putchar('\n');
    }
}

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

#ifndef IMG_AT
#define IMG_AT(img, x, y) &(img)[(y)*w*comp + (x)*comp]
#endif // IMG_AT

int w, h, comp;

u8 *image_at(u8 *image, int x, int y)
{
    return &image[y*w*comp + x*comp];
}

int main(int argc, char **argv)
{
    if (argc < 2) error("Expected input image filename\n");
    const char *filename_in = argv[1];
    if (argc < 4) error("Expected 2 output image filenames\n");
    const char *filename1 = argv[2];
    const char *filename2 = argv[3];

    u8 *image_in = stbi_load(filename_in, &w, &h, &comp, STBI_rgb);
    if (image_in == NULL) {
        error("Failed to open file \"%s\"\n", argv[1]);
    }
    u8 *image1 = malloc(w * h * comp);
    memset(image1, 0xff, w * h * comp);
    u8 *image2 = malloc(w * h * comp);
    memset(image2, 0xff, w * h * comp);

    table_t *fg1 = table_new(5, 5);
    table_t *fg2 = table_new(5, 5);
    const float dg1 = 10.f;
    const float dg2 = 20.f;
    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < 5; col++) {
            float x = (float)(row - 2);
            float y = (float)(col - 2);
            *table_at(fg1, row, col) = 1.f / sqrtf(2*PI*dg1*dg1) * expf(-(x*x+y*y)/(2*dg1*dg1));
            *table_at(fg2, row, col) = 1.f / sqrtf(2*PI*dg2*dg2) * expf(-(x*x+y*y)/(2*dg2*dg2));
        }
    }

    for (int x = 0; x < w; x++) {
        for (int y = 0; y < h; y++) {
            u8 *in = image_at(image_in, x, y);
            u8 res[3] = { 0 };
            for (int dx = -2; dx <= 2; dx++) {
                for (int dy = -2; dy <= 2; dy++) {
                    int cx = x + dx;
                    int cy = x + dy;
                    if (cx < 0 || cx >= w || cy < 0 || cy >= h) continue;
                    res[0] = MIN((int)res[0] + *table_at(fg1, dx + 2, dy + 2) * (float)in[0], 255);
                    res[1] = MIN((int)res[1] + *table_at(fg1, dx + 2, dy + 2) * (float)in[1], 255);
                    res[2] = MIN((int)res[2] + *table_at(fg1, dx + 2, dy + 2) * (float)in[2], 255);
                    res[0] = MAX((int)res[0] - *table_at(fg2, dx + 2, dy + 2) * (float)in[0], 0);
                    res[1] = MAX((int)res[1] - *table_at(fg2, dx + 2, dy + 2) * (float)in[1], 0);
                    res[2] = MAX((int)res[2] - *table_at(fg2, dx + 2, dy + 2) * (float)in[2], 0);
                }
            }
            memmove(image_at(image1, x, y), res, 3);
        }
    }

    table_t *fr = table_new(2, 2);
    memmove(fr->data, &(float[]){ 1.f, 0.f, 0.f, -1.f }, 4 * sizeof(float));
    table_t *fs = table_new(3, 3);
    memmove(fs->data, &(float[]){-1.f, 0.f, 1.f, -2.f, 0.f, 2.f, -1.f, 0.f, 1.f }, 9 * sizeof(float));

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            u8 *in = image_at(image_in, x, y);
            u8 res[3] = { 0 };
            if (x < w/2) {
                for (int dx = 0; dx <= 1; dx++) {
                    for (int dy = 0; dy <= 1; dy++) {
                        int cx = x + dx;
                        int cy = x + dy;
                        if (cx < 0 || cx >= w || cy < 0 || cy >= h) continue;
                        res[0] = MAX(MIN((int)res[0] + *table_at(fr, dx, dy) * (float)in[0], 255), 0);
                        res[1] = MAX(MIN((int)res[1] + *table_at(fr, dx, dy) * (float)in[1], 255), 0);
                        res[2] = MAX(MIN((int)res[2] + *table_at(fr, dx, dy) * (float)in[2], 255), 0);
                    }
                }
            } else {
                for (int dx = -1; dx <= 1; dx++) {
                    for (int dy = -1; dy <= 1; dy++) {
                        int cx = x + dx;
                        int cy = x + dy;
                        if (cx < 0 || cx >= w || cy < 0 || cy >= h) continue;
                        res[0] = MAX(MIN((int)res[0] + *table_at(fs, dx + 1, dy + 1) * (float)in[0], 255), 0);
                        res[1] = MAX(MIN((int)res[1] + *table_at(fs, dx + 1, dy + 1) * (float)in[1], 255), 0);
                        res[2] = MAX(MIN((int)res[2] + *table_at(fs, dx + 1, dy + 1) * (float)in[2], 255), 0);
                    }
                }
            }
            memmove(image_at(image2, x, y), res, 3);
        }
    }

    write_image(filename1, w, h, comp, image1);
    write_image(filename2, w, h, comp, image2);
    table_free(fg1);
    table_free(fg2);
    table_free(fr);
    table_free(fs);
    free(image1);
    free(image2);
}
