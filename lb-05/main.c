#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "stb_image_write.h"

#define ARR_LEN(arr) (sizeof(arr) / sizeof(*arr))

#define IMG_W    100
#define IMG_H    100
#define IMG_COMP 4

void draw_line(uint8_t *img, int img_w, int img_h,
               int x1, int y1, int x2, int y2, uint32_t clr);

uint8_t img[IMG_W * IMG_H * IMG_COMP] = { 0 };

typedef struct {
    float x, y, z;
} vec3;

vec3 tetrahedron_points[] = {
    { -1.75, 1.5, 3.5 },
    {  1.5,  1.5, 3   },
    { -0.5, -1.5, 2   },
    { -0.5,  1.5, 2   },
};

size_t tetrahedron_edges[] = {
    0,1, 1,3, 3,0,
    0,2, 1,2, 3,2,
};

vec3 octahedron_points[] = {
    { -0.5,  0.5, 2 },
    {  0.5, -0.5, 4 },
    { -1.5,    0, 2 },
    {  1.5,    0, 2 },
    {  0,   -1.5, 2 },
    {  0,    1.5, 2 },
};

size_t octahedron_edges[] = {
    0,2, 2,1, 1,3, 3,0,
    0,4, 4,1, 1,5, 5,0,
    2,4, 4,3, 3,5, 5,2,
};

void get_screen_coords(vec3 point, int *x, int *y)
{
    float fx, fy;
    fx = point.x / point.z;
    *x = IMG_W/2 * (fx + 1);
    fy = point.y / point.z;
    *y = IMG_H/2 * (fy + 1);
}

void draw_edge(size_t edge_start, size_t edge_end,
               vec3 *points, uint32_t clr)
{
    vec3 point_start = points[edge_start];
    vec3 point_end = points[edge_end];
    int x1, y1, x2, y2;
    get_screen_coords(point_start, &x1, &y1);
    get_screen_coords(point_end, &x2, &y2);
    draw_line(img, IMG_W, IMG_H, x1, y1, x2, y2, clr);
}

void generate_figure(const char *filename, vec3 *points,
                     size_t *edges, size_t edges_n)
{
    const uint32_t clr = 0xff000000;
    memset(img, 0xff, sizeof(img));
    for (size_t i = 0; i < edges_n; i += 2) {
        draw_edge(edges[i], edges[i+1], points, clr);
    }
    stbi_write_png(filename, IMG_W, IMG_H, IMG_COMP, img, IMG_W * IMG_COMP);
    printf("Wrote to \"%s\"\n", filename);
}

int main(void)
{
    generate_figure("tetrahedron.png", tetrahedron_points,
                    tetrahedron_edges, ARR_LEN(tetrahedron_edges));
    generate_figure("octahedron.png", octahedron_points,
                    octahedron_edges, ARR_LEN(octahedron_edges));
}
