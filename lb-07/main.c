#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#include "stb_image.h"
#include "stb_image_write.h"

typedef uint8_t  u8;
typedef uint32_t u32;

#define error(...)                    \
    do {                              \
        fprintf(stderr, __VA_ARGS__); \
        exit(1);                      \
    } while (0)

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

u8 *image_in = NULL;
u8 *image_out = NULL;
int w, h, comp;

#ifndef IMG_AT
#define IMG_AT(img, x, y) &(img)[(y)*w*comp + (x)*comp]
#endif // IMG_AT

int main(int argc, char **argv)
{
    if (argc < 2) {
        error("Expected input image filename\n");
    }
    if (argc < 3) {
        error("Expected output image filename\n");
    }

    image_in = stbi_load(argv[1], &w, &h, &comp, STBI_rgb);
    if (image_in == NULL) {
        error("Failed to open file \"%s\"\n", argv[1]);
    }
    image_out = malloc(w*h*comp);
    memset(image_out, 0xff, w*h*comp);
    
    const int rw1 = w/4, rh1 = h/4;
    const int x1 = w/2 - rw1/2, x2 = x1 + rw1;
    const int y1 = h/2 - rh1/2, y2 = y1 + rw1;

    for (int x = x1; x <= x2; x++) {
        for (int y = y1; y <= y2; y++) {
            *IMG_AT(image_out, x, y)       = *IMG_AT(image_in, x, y);
            *(IMG_AT(image_out, x, y) + 1) = *(IMG_AT(image_in, x, y) + 1);
            *(IMG_AT(image_out, x, y) + 2) = *(IMG_AT(image_in, x, y) + 2);
        }
    }

    const u32 clr_graph = 0xff0000ff;
    const u32 clr_axes = 0xff000000;

    line(image_out, 0, h/2, w, h/2, w, h, comp, clr_axes);
    line(image_out, w/2, 0, w/2, h, w, h, comp, clr_axes);

    float px = 0;
    float py = expf(px - w/2);

    for (int x = 1; x < w; x++) {
        int y = expf(x - w/2);
        line(image_out, (int)px, (int)(h/2 - py), x, (int)(h/2 - y), w, h, comp, clr_graph);
        px = x;
        py = expf(px - w/2);
        if (y > h/2) break;
    }

    write_image(argv[2], w, h, comp, image_out);
    free(image_out);
}
