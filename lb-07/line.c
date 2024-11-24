#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef unsigned char u8;
typedef unsigned int u32;

#ifndef IMG_AT
#define IMG_AT(img, x, y) &(img)[(y)*w*comp + (x)*comp]
#endif // IMG_AT

#define DRAW_AT(img, x, y, clr)                               \
    do {                                                      \
        int start = (y)*w*comp + (x)*comp;                    \
        if (start < 0) break;                                 \
        if (start > w*h*comp) break;                          \
        *IMG_AT(img, x, y) = (u8)((clr) & 0xff);              \
        if (start + 1 > w*h*comp) break;                      \
        *(IMG_AT(img, x, y) + 1) = (u8)((clr) >> 8 & 0xff);   \
        if (start + 2 > w*h*comp) break;                      \
        *(IMG_AT(img, x, y) + 2) = (u8)((clr) >> 16 & 0xff);  \
    } while (0)

void line(u8 *img,
          int x1, int y1, int x2, int y2,
          int w, int h, int comp, u32 clr)
{
    assert(img != NULL);
    assert(w > 0);
    assert(h > 0);

    float dx = abs(x2 - x1);
    float dy = abs(y2 - y1);

    if (x1 == x2 && y1 == y2) {
        DRAW_AT(img, x1, y1, clr);
        return;
    }

    float l = fmaxf(dx, dy);
    dx = (x2 - x1) / l;
    dy = (y2 - y1) / l;

    float x = (float)x1;
    float y = (float)y1;

    for (int i = 0; i < l; i++) {
        if (x >= 0 && y >= 0 && x < w && y < h) {
            DRAW_AT(img, (int)x, (int)y, clr);
        }
        x += dx;
        y += dy;
        if (x > w || y > h || x < 0 || y < 0) break;
    }
}
