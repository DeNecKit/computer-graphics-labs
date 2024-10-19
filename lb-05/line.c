#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define SIGN(x) ((x) > 0 ? 1 : (x) < 0 ? -1 : 0)
#define ABS(x) ((x) < 0 ? -(x) : (x))

void draw_line(uint8_t *img, int img_w, int img_h,
               int x1, int y1, int x2, int y2, uint32_t clr)
{
    assert(img != NULL);
    assert(img_w > 0);
    assert(img_h > 0);
    assert(x1 >= 0 && y1 >= 0 && x1 < img_w && y1 < img_h);
    assert(x2 >= 0 && y2 >= 0 && x2 < img_w && y2 < img_h);
    int dx = x2 - x1;
    int dy = y2 - y1;
    int sx = SIGN(dx);
    int sy = SIGN(dy);
    dx = ABS(dx);
    dy = ABS(dy);

    if (x1 == x2 && y1 == y2) {
        ((uint32_t*)img)[y1*img_w + x1] = clr;
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
        if ((sx > 0 && x > x2) ||
            (sx < 0 && x < x2) ||
            (sy > 0 && y > y2) ||
            (sy < 0 && y < y2)) break;
        ((uint32_t*)img)[y*img_w + x] = clr;
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
}
