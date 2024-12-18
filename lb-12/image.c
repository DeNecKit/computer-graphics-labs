#include <math.h>
#include <string.h>

#include "image.h"
#include "util.h"
#include "stb_image_write.h"

u8 *image_at(const image_t *image, int x, int y)
{
    int w = image->w;
    u8 *data = image->data;
    return data + (y*w + x) * 4;
}

bool image_is_color(const image_t *image, int x, int y, u32 color)
{
    return *(u32*)image_at(image, x, y) == color;
}

void image_set(image_t *image, int x, int y, u32 color)
{
    *(u32*)image_at(image, x, y) = color;
}

void image_line(image_t *image, int x1, int y1, int x2, int y2, u32 color)
{
    assert(image != NULL);
    assert(x1 >= 0);
    assert(y1 >= 0);
    assert(x2 >= 0);
    assert(y2 >= 0);

    float dx = abs(x2 - x1);
    float dy = abs(y2 - y1);

    if (x1 == x2 && y1 == y2) {
        image_set(image, x1, y1, color);
        return;
    }

    float l = fmaxf(dx, dy);
    dx = (x2 - x1) / l;
    dy = (y2 - y1) / l;

    float x = (float)x1;
    float y = (float)y1;

    for (int i = 0; i < l; i++) {
        if (x >= 0 && y >= 0 && x < image->w && y < image->h) {
            image_set(image, (int)x, (int)y, color);
        }
        x += dx;
        y += dy;
    }
}

const char *get_file_ext(const char *filename)
{
    while (*filename && *filename != '.') filename++;
    return filename;
}

void image_fill(image_t *image, int x, int y, u32 color)
{
    assert(image != NULL);
    assert(x >= 0);
    assert(y >= 0);
    assert(x < image->w);
    assert(y < image->h);

    stack_t *stack = NULL;

    if (!image_is_color(image, x, y, color)) {
        vec2 *start = malloc(sizeof(vec2));
        *start = (vec2) { x, y };
        push(&stack, start);
    }

    while (stack != NULL) {
        vec2 *vmem = pop(&stack);
        vec2 v = *vmem;
        free(vmem);
        image_set(image, v.x, v.y, color);

        vec2 v1 = { v.x - 1, v.y };
        if (v1.x >= 0 && !image_is_color(image, v1.x, v1.y, color)) {
            vec2 *v1mem = malloc(sizeof(vec2));
            *v1mem = v1;
            push(&stack, v1mem);
        }

        vec2 v2 = { v.x, v.y - 1 };
        if (v2.y >= 0 && !image_is_color(image, v2.x, v2.y, color)) {
            vec2 *v2mem = malloc(sizeof(vec2));
            *v2mem = v2;
            push(&stack, v2mem);
        }

        vec2 v3 = { v.x + 1, v.y };
        if (v3.x < image->w && !image_is_color(image, v3.x, v3.y, color)) {
            vec2 *v3mem = malloc(sizeof(vec2));
            *v3mem = v3;
            push(&stack, v3mem);
        }

        vec2 v4 = { v.x, v.y + 1 };
        if (v4.y < image->h && !image_is_color(image, v4.x, v4.y, color)) {
            vec2 *v4mem = malloc(sizeof(vec2));
            *v4mem = v4;
            push(&stack, v4mem);
        }
    }
}

void image_write(const char *filename, const image_t *image)
{
    int w = image->w;
    int h = image->h;
    int comp = 4;
    u8 *data = image->data;
    const char *ext = get_file_ext(filename);
    int res = 1;
    if (strcmp(ext, ".jpg") == 0) {
        res = stbi_write_jpg(filename, w, h, comp, data, 100);
    } else if (strcmp(ext, ".png") == 0) {
        res = stbi_write_png(filename, w, h, comp, data, w * comp);
    } else {
        error(L"Расширение файла не поддерживается: \"%s\"\n", ext);
    }
    if (res) {
        wprintf(L"Файл \"%S\" сохранён\n", filename);
    } else {
        error(L"Не удалось сохранить файл \"%S\"\n", filename);
    }
}
