#include <assert.h>
#include <stddef.h>
#include <string.h>
#include <math.h>

typedef unsigned char u8;
typedef unsigned int u32;

void line(void *img,
          float x1, float y1, float x2, float y2,
          int w, int h, u32 clr)
{
    assert(img != NULL);
    assert(w > 0);
    assert(h > 0);

    float dx = fabsf(x2 - x1);
    float dy = fabsf(y2 - y1);

    if (x1 == x2 && y1 == y2) {
        ((u32*)img)[(int)(y1*w + x1)] = clr;
        return;
    }

    float l = fmaxf(dx, dy);
    dx = (x2 - x1) / l;
    dy = (y2 - y1) / l;

    float x = x1;
    float y = y1;

    for (int i = 0; i < l; i++) {
        ((u32*)img)[(int)(y*w + x)] = clr;
        x += dx;
        y += dy;
    }
    ((u32*)img)[(int)(y2*w + x2)] = clr;
}
