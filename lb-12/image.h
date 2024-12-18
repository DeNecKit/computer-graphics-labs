#ifndef IMAGE_H
#define IMAGE_H

#include <stdint.h>
#include <stdbool.h>

typedef uint8_t  u8;
typedef uint32_t u32;

typedef struct {
    u8 *data;
    int w, h;
} image_t;

u8 *image_at(const image_t *image, int x, int y);
bool image_is_color(const image_t *image, int x, int y, u32 color);
void image_set(image_t *image, int x, int y, u32 color);
void image_line(image_t *image, int x1, int y1, int x2, int y2, u32 color);
void image_fill(image_t *image, int x, int y, u32 color);
void image_write(const char *filename, const image_t *image);


#endif // IMAGE_H
