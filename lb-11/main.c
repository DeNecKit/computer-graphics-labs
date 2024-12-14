#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "stb_image_write.h"

#define error(...)                    \
    do {                              \
        fprintf(stderr, __VA_ARGS__); \
        exit(1);                      \
    } while (0)

#define ARR_LEN(arr) ((int)(sizeof(arr) / sizeof(*(arr))))

typedef uint8_t  u8;
typedef uint32_t u32;

typedef struct {
    int x, y;
} vec2;

typedef struct {
    void *items;
    int item_size;
    int len;
} array_t;

array_t *array_new(int len, int item_size)
{
    assert(len > 0);
    assert(item_size > 0);
    array_t *res = malloc(sizeof(*res));
    res->items = calloc(len, item_size);
    res->item_size = item_size;
    res->len = len;
    return res;
}

void *array_at(const array_t *arr, int idx)
{
    assert(arr != NULL);
    assert(idx >= 0);
    assert(idx < arr->len);
    return (char*)arr->items + idx * arr->item_size;
}

array_t *array_add(const array_t *arr, void *item)
{
    assert(arr != NULL);
    array_t *res = malloc(sizeof(*res));
    *res = (array_t) {
        .items = calloc(arr->len + 1, arr->item_size),
        .item_size = arr->item_size,
        .len = arr->len + 1
    };
    memmove(res->items, arr->items, arr->len * arr->item_size);
    memmove(((char*)res->items + arr->len * arr->item_size), item, arr->item_size);
    return res;
}

array_t *array_copy(const array_t *arr)
{
    array_t *res = malloc(sizeof(*res));
    *res = (array_t) {
        .items = calloc(arr->len, arr->item_size),
        .item_size = arr->item_size,
        .len = arr->len
    };
    memmove(res->items, arr->items, arr->len * arr->item_size);
    return res;
}

void array_free(array_t *arr)
{
    free(arr->items);
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

int w = 500;
int h = 500;
int comp = 3;

#define BLACK ((u32)0xff000000)
#define GRAY  ((u32)0xff808080)

u8 *image_at(u8 *image, int x, int y)
{
    return &image[y*w*comp + x*comp];
}

void line(u8 *img,
          int x1, int y1, int x2, int y2,
          int w, int h, int comp, u32 clr);

vec2 cubic_bezier(float t, vec2 p0, vec2 p1, vec2 p2, vec2 p3)
{
    vec2 res;
    res.x = powf(1-t,3)*p0.x + 3*t*powf(1-t,2)*p1.x + 3*(1-t)*t*t*p2.x + t*t*t*p3.x;
    res.y = powf(1-t,3)*p0.y + 3*t*powf(1-t,2)*p1.y + 3*(1-t)*t*t*p2.y + t*t*t*p3.y;
    return res;
}

void task1(u8 *image)
{
    vec2 curve_points_arr[] = {
        { 250, 450 },
        { 50 , 250 },
        { 450, 250 },
        { 250, 50 }
    };
    array_t *curve_points = array_new(
        ARR_LEN(curve_points_arr), sizeof(vec2));
    memmove(curve_points->items, curve_points_arr,
        sizeof(vec2) * ARR_LEN(curve_points_arr));

    for (int i = 0; i < curve_points->len - 1; i++) {
        vec2 p1 = *(vec2*)array_at(curve_points, i);
        vec2 p2 = *(vec2*)array_at(curve_points, i + 1);
        line(image, p1.x, p1.y, p2.x, p2.y, w, h, comp, GRAY);
    }

    for (int j = 0; j < 3; j++) {
        array_t *new_curve_points = array_new(
            (curve_points->len - 1) * 2, curve_points->item_size);
        for (int i = 0; i < curve_points->len - 1; i++) {
            vec2* p1 = (vec2*)array_at(curve_points, i);
            vec2* p2 = (vec2*)array_at(curve_points, i+1);
            vec2* np1 = (vec2*)array_at(new_curve_points, i*2);
            vec2* np2 = (vec2*)array_at(new_curve_points, i*2+1);
            np1->x = 3.f/4.f*p1->x + 1.f/4.f*p2->x;
            np1->y = 3.f/4.f*p1->y + 1.f/4.f*p2->y;
            np2->x = 3.f/4.f*p2->x + 1.f/4.f*p1->x;
            np2->y = 3.f/4.f*p2->y + 1.f/4.f*p1->y;
        }
        array_free(curve_points);
        curve_points = new_curve_points;
    }

    for (int i = 0; i < curve_points->len - 1; i++) {
        vec2 p1 = *(vec2*)array_at(curve_points, i);
        vec2 p2 = *(vec2*)array_at(curve_points, i + 1);
        line(image, p1.x, p1.y, p2.x, p2.y, w, h, comp, BLACK);
    }
}

void task2(u8 *image)
{
    const int bezier_n = 4;
    const int bezier_m = 4;
    array_t *bezier_points = array_new(bezier_n * bezier_m, sizeof(vec2));
    memmove(bezier_points->items,
        (vec2[]) {
            { 125, 100 }, { 200, 75 } , { 300, 100 }, { 425, 75 } ,
            { 75, 175 } , { 200, 225 }, { 275, 200 }, { 375, 175 },
            { 75, 325  }, { 225, 300 }, { 300, 275 }, { 350, 300 },
            { 125, 400 }, { 175, 425 }, { 275, 375 }, { 400, 425 }
        }, bezier_n * bezier_m * sizeof(vec2));
    
    for (int x = 0; x < bezier_n; x++) {
        for (int y = 0; y < bezier_n - 1; y++) {
            vec2 p1 = *(vec2*)array_at(bezier_points, y*bezier_n + x);
            vec2 p2 = *(vec2*)array_at(bezier_points, (y+1)*bezier_n + x);
            line(image, p1.x, p1.y, p2.x, p2.y, w, h, comp, GRAY);
        }
    }
    for (int x = 0; x < bezier_n - 1; x++) {
        for (int y = 0; y < bezier_n; y++) {
            vec2 p1 = *(vec2*)array_at(bezier_points, y*bezier_n + x);
            vec2 p2 = *(vec2*)array_at(bezier_points, y*bezier_n + x + 1);
            line(image, p1.x, p1.y, p2.x, p2.y, w, h, comp, GRAY);
        }
    }

    const vec2 bezier1[] = {
        *(vec2*)array_at(bezier_points, 0),
        *(vec2*)array_at(bezier_points, 1),
        *(vec2*)array_at(bezier_points, 2),
        *(vec2*)array_at(bezier_points, 3)
    };
    const vec2 bezier2[] = {
        *(vec2*)array_at(bezier_points, bezier_n*(bezier_m-1)),
        *(vec2*)array_at(bezier_points, bezier_n*(bezier_m-1) + 1),
        *(vec2*)array_at(bezier_points, bezier_n*(bezier_m-1) + 2),
        *(vec2*)array_at(bezier_points, bezier_n*(bezier_m-1) + 3)
    };
    const vec2 bezier3[] = {
        *(vec2*)array_at(bezier_points, 0),
        *(vec2*)array_at(bezier_points, bezier_n),
        *(vec2*)array_at(bezier_points, 2*bezier_n),
        *(vec2*)array_at(bezier_points, 3*bezier_n)
    };
    const vec2 bezier4[] = {
        *(vec2*)array_at(bezier_points, bezier_n - 1),
        *(vec2*)array_at(bezier_points, 2*bezier_n - 1),
        *(vec2*)array_at(bezier_points, 3*bezier_n - 1),
        *(vec2*)array_at(bezier_points, 4*bezier_n - 1)
    };
    vec2 prev_p1 = bezier1[0];
    vec2 prev_p2 = bezier2[0];
    vec2 prev_p3 = bezier3[0];
    vec2 prev_p4 = bezier4[0];
    for (float t = 0.f; t <= 1.f; t += 0.001f) {
        vec2 p1 = cubic_bezier(t, bezier1[0], bezier1[1], bezier1[2], bezier1[3]);
        vec2 p2 = cubic_bezier(t, bezier2[0], bezier2[1], bezier2[2], bezier2[3]);
        vec2 p3 = cubic_bezier(t, bezier3[0], bezier3[1], bezier3[2], bezier3[3]);
        vec2 p4 = cubic_bezier(t, bezier4[0], bezier4[1], bezier4[2], bezier4[3]);
        line(image, prev_p1.x, prev_p1.y, p1.x, p1.y, w, h, comp, BLACK);
        // line(image, prev_p1.x, prev_p1.y + 1, p1.x, p1.y + 1, w, h, comp, BLACK);
        line(image, prev_p2.x, prev_p2.y, p2.x, p2.y, w, h, comp, BLACK);
        // line(image, prev_p2.x, prev_p2.y + 1, p2.x, p2.y + 1, w, h, comp, BLACK);
        line(image, prev_p3.x, prev_p3.y, p3.x, p3.y, w, h, comp, BLACK);
        // line(image, prev_p3.x + 1, prev_p3.y, p3.x + 1, p3.y, w, h, comp, BLACK);
        line(image, prev_p4.x, prev_p4.y, p4.x, p4.y, w, h, comp, BLACK);
        // line(image, prev_p4.x + 1, prev_p4.y, p4.x + 1, p4.y, w, h, comp, BLACK);
        prev_p1 = p1;
        prev_p2 = p2;
        prev_p3 = p3;
        prev_p4 = p4;
    }
}

void task3(u8 *image)
{
    vec2 polygon[] = {
        { 100, 100 },
        { 450, 50 },
        { 400, 400 },
        { 50, 450 }
    };

    for (int i = 0; i < ARR_LEN(polygon); i++) {
        vec2 p1 = polygon[i];
        vec2 p2 = polygon[(i+1) % ARR_LEN(polygon)];
        line(image, p1.x, p1.y, p2.x, p2.y, w, h, comp, BLACK);
    }

    vec2 center = { 0 };
    for (int i = 0; i < ARR_LEN(polygon); i++) {
        center.x += polygon[i].x;
        center.y += polygon[i].y;
    }
    center.x /= ARR_LEN(polygon);
    center.y /= ARR_LEN(polygon);

    vec2 line_centers[ARR_LEN(polygon)] = { 0 };
    for (int i = 0; i < ARR_LEN(polygon); i++) {
        vec2 p1 = polygon[i];
        vec2 p2 = polygon[(i+1) % ARR_LEN(polygon)];
        line_centers[i] = (vec2) {
            .x = (p1.x + p2.x) / 2,
            .y = (p1.y + p2.y) / 2
        };
    }

    for (int i = 0; i < ARR_LEN(polygon); i++) {
        polygon[i] = (vec2) {
            .x = (center.x + polygon[i].x + line_centers[i].x + line_centers[(i-1+ARR_LEN(line_centers)) % ARR_LEN(line_centers)].x) / 4,
            .y = (center.y + polygon[i].y + line_centers[i].y + line_centers[(i-1+ARR_LEN(line_centers)) % ARR_LEN(line_centers)].y) / 4,
        };
    }

    for (int i = 0; i < ARR_LEN(polygon); i++) {
        vec2 p1 = polygon[i];
        vec2 p2 = polygon[(i+1) % ARR_LEN(polygon)];
        line(image, p1.x, p1.y, p2.x, p2.y, w, h, comp, BLACK);
    }
}

int main(int argc, char **argv)
{
    if (argc < 4) error("Expected 3 output image filenames\n");
    const char *filename1 = argv[1];
    const char *filename2 = argv[2];
    const char *filename3 = argv[3];

    u8 *image1 = malloc(w * h * comp);
    memset(image1, 0xff, w * h * comp);
    u8 *image2 = malloc(w * h * comp);
    memset(image2, 0xff, w * h * comp);
    u8 *image3 = malloc(w * h * comp);
    memset(image3, 0xff, w * h * comp);

    task1(image1);
    task2(image2);
    task3(image3);

    write_image(filename1, w, h, comp, image1);
    write_image(filename2, w, h, comp, image2);
    write_image(filename3, w, h, comp, image3);

    free(image1);
    free(image2);
    free(image3);
}
