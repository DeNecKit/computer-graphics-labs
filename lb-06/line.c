#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef unsigned char u8;
typedef unsigned int u32;

void line(void *img,
          int x1, int y1, int x2, int y2,
          int w, int h, u32 clr)
{
    assert(img != NULL);
    assert(w > 0);
    assert(h > 0);

    float dx = abs(x2 - x1);
    float dy = abs(y2 - y1);

#define DRAW(x, y)                                        \
    do {                                                  \
        if ((x) >= 0 && (y) >= 0 && (x) < w && (y) < h) { \
            ((u32*)img)[(int)(y)*w + (int)(x)] = clr;     \
        }                                                 \
    } while (0)

    if (x1 == x2 && y1 == y2) {
        DRAW(x1, y1);
        return;
    }

    float l = fmaxf(dx, dy);
    dx = (x2 - x1) / l;
    dy = (y2 - y1) / l;

    float x = (float)x1;
    float y = (float)y1;

    for (int i = 0; i < l; i++) {
        DRAW(x, y);
        x += dx;
        y += dy;
    }
    DRAW(x2, y2);
}
