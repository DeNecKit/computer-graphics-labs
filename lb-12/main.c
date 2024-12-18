#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <wchar.h>

#include "util.h"
#include "image.h"

void save();

#define BLACK      ((u32)0xff000000)
#define GRAY       ((u32)0xff808080)
#define DARK_GRAY  ((u32)0xff000000)

int parse_int(void)
{
    int res;
    if (scanf("%d*c", &res) != 1) {
        error(L"Не удалось прочитать число\n");
    }
    return res;
}

int window_x;
int window_y;
int window_width;
int window_height;
int scale;

char filename[1024] = { 0 };
image_t *image, *overlay_image;

array_t *polygons;
u32 *colors;

enum { OUTSIDE, INSIDE, CROSSING, COVERING };

enum { LEFT_TOP,    TOP,    RIGHT_TOP,
       LEFT,        CENTER, RIGHT,
       LEFT_BOTTOM, BOTTOM, RIGHT_BOTTOM };

int calc_polygon_pos(array_t *polygon, vec2 wp0, vec2 wp1)
{
    int xmin = INT_MAX, ymin = INT_MAX, xmax = INT_MIN, ymax = INT_MIN;
    for (int i = 0; i < polygon->len; i++) {
        vec3 v = *(vec3*)array_at(polygon, i);
        if (v.x < xmin) xmin = v.x;
        if (v.x > xmax) xmax = v.x;
        if (v.y < ymin) ymin = v.y;
        if (v.x > ymax) ymax = v.y;
    }

    if (xmin >= wp0.x && xmax <= wp1.x &&
        ymin >= wp0.y && ymax <= wp1.y) return INSIDE;

    for (int i = 0; i < polygon->len; i++) {
        vec3 v = *(vec3*)array_at(polygon, i);
        if (v.x >= wp0.x && v.x <= wp1.x &&
            v.y >= wp0.y && v.y <= wp1.y) return CROSSING;
    }

    if (xmin < wp0.x && xmax > wp1.x && ymin < wp0.y && ymax > wp1.y) {
        return COVERING;
    }

    if (xmax < wp0.x || xmin > wp1.x ||
        ymax < wp0.y || ymin > wp1.y) return OUTSIDE;

    return CROSSING;
}

void draw_axes()
{
    int wx0 = window_x * scale;
    int wy0 = window_y * scale;
    int wx1 = (window_x + window_width + 1) * scale;
    int wy1 = (window_y + window_height + 1) * scale;

    image_line(overlay_image, wx0, wy0, wx1, wy0, GRAY);
    for (int x = window_x; x <= window_width + 1; x++) {
        int wx = x * scale;
        image_line(overlay_image, wx, wy0 - scale/4, wx, wy0 + scale/4, GRAY);
    }

    image_line(overlay_image, wx0, wy0, wx0, wy1, GRAY);
    for (int y = window_y; y <= window_height + 1; y++) {
        int wy = y * scale;
        image_line(overlay_image, wx0 - scale/4, wy, wx0 + scale/4, wy, GRAY);
    }
}

void draw_pixel(vec2 wp0, vec2 wp1)
{
    int closest_i = -1;
    int closest_z = INT_MAX;
    for (int i = 0; i < polygons->len; i++) {
        if (calc_polygon_pos(array_at(polygons, i), wp0, wp1) == OUTSIDE) continue;
        int far_z = INT_MIN;
        array_t *polygon = array_at(polygons, i);
        for (int j = 0; j < polygon->len; j++) {
            vec3 v = *(vec3*)array_at(polygon, j);
            if (v.z > far_z) far_z = v.z;
        }
        if (far_z < closest_z) {
            closest_i = i;
            closest_z = far_z;
        }
    }

    if (closest_i == -1) return;

    int x0 = (window_x + wp0.x) * scale;
    int y0 = (window_y + wp0.y) * scale;
    int x1 = (window_x + wp1.x) * scale;
    int y1 = (window_y + wp1.y) * scale;
    for (int x = x0; x < x1; x++) {
        for (int y = y0; y < y1; y++) {
            image_set(image, x, y, colors[closest_i]);
        }
    }
}

void draw_at(vec2 wp0, vec2 wp1)
{
    int polygon_i = 0;
    for (int i = 1; i < polygons->len; i++) {
        if (calc_polygon_pos(array_at(polygons, i), wp0, wp1) != OUTSIDE) {
            polygon_i = i;
            break;
        }
    }

    array_t *polygon = array_at(polygons, polygon_i);
    vec3 new_polygon[polygon->len];
    for (int i = 0; i < polygon->len; i++) {
        vec3 v = *(vec3*)array_at(polygon, i);
        new_polygon[i] = (vec3) { v.x, v.y, v.z };
        if (new_polygon[i].x > wp1.x) new_polygon[i].x = wp1.x;
        if (new_polygon[i].y > wp1.y) new_polygon[i].y = wp1.y;
        if (new_polygon[i].x < wp0.x) new_polygon[i].x = wp0.x;
        if (new_polygon[i].y < wp0.y) new_polygon[i].y = wp0.y;
    }

    image_t tmp_image;
    tmp_image.w = image->w;
    tmp_image.h = image->h;
    tmp_image.data = calloc(image->w * image->h, 4);

    vec2 cv = { 0 };
    for (int i = 0; i < polygon->len; i++) {
        vec3 v1 = new_polygon[i];
        vec3 v2 = new_polygon[(i + 1) % polygon->len];
        image_line(&tmp_image,
            (window_x + v1.x) * scale, (window_y + v1.y) * scale,
            (window_x + v2.x) * scale, (window_y + v2.y) * scale,
            colors[polygon_i]);
        cv.x += (window_x + v1.x) * scale;
        cv.y += (window_y + v1.y) * scale;
    }
    cv.x /= polygon->len;
    cv.y /= polygon->len;
    image_fill(&tmp_image, cv.x, cv.y, colors[polygon_i]);

    for (int x = 0; x < image->w; x++) {
        for (int y = 0; y < image->h; y++) {
            if (*(u32*)image_at(&tmp_image, x, y) == colors[polygon_i]) {
                image_set(image, x, y, *(u32*)image_at(&tmp_image, x, y));
            }
        }
    }
}

void draw_window(vec2 wp0, vec2 wp1)
{
    int x0 = (window_x + wp0.x) * scale;
    int y0 = (window_y + wp0.y) * scale;
    int x1 = (window_x + wp1.x) * scale;
    int y1 = (window_y + wp1.y) * scale;

    image_line(overlay_image, x0, y0, x1, y0, DARK_GRAY);
    image_line(overlay_image, x0, y1, x1, y1, DARK_GRAY);
    image_line(overlay_image, x0, y0, x0, y1, DARK_GRAY);
    image_line(overlay_image, x1, y0, x1, y1, DARK_GRAY);
}

void draw_varnok(vec2 wp0, vec2 wp1, bool show_steps)
{
    if (wp0.x == wp1.x || wp0.y == wp1.y) return;

    if (show_steps) {
        memset(overlay_image->data, 0x00, overlay_image->w * overlay_image->h * 4);
    }

    int polygons_pos[polygons->len];
    int inside = 0;
    for (int i = 0; i < polygons->len; i++) {
        polygons_pos[i] = calc_polygon_pos((array_t*)array_at(polygons, i), wp0, wp1);
        if (polygons_pos[i] != OUTSIDE) {
            inside++;
        }
    }

    bool quit = false;

    if (inside == 1)  {
        draw_at(wp0, wp1);
        quit = true;
    } else if (wp1.x - wp0.x == 1 && wp1.y - wp0.y == 1) {
        draw_pixel(wp0, wp1);
        quit = true;
    }

    if (show_steps) {
        draw_axes();
        draw_window(wp0, wp1);

        for (int x = 0; x < overlay_image->w; x++) {
            for (int y = 0; y < overlay_image->h; y++) {
                if (image_is_color(overlay_image, x, y, 0)) {
                    image_set(overlay_image, x, y, *(u32*)image_at(image, x, y));
                }
            }
        }

        image_write(filename, overlay_image);
        wprintf(L"\nШаг алгоритма сохранён в файл \"%S\"\n", filename);
        wprintf(L"Положение многоугольников:\n");
        for (int i = 0; i < polygons->len; i++) {
            const wchar_t *state = NULL;
            switch (polygons_pos[i]) {
                case OUTSIDE: state = L"вне окна"; break;
                case INSIDE: state = L"внутри окна"; break;
                case CROSSING: state = L"пересекает окно"; break;
                case COVERING: state = L"охватывает окно"; break;
                default: assert(false && "Unreachable");
            }
            wprintf(L"Многоугольник №%d %ls\n", i+1, state);
        }
        wprintf(L"  (нажмите ENTER для продолжения)");
        getchar();
    }

    if (quit) return;

    int dx = wp1.x - wp0.x;
    int dy = wp1.y - wp0.y;
    if (dx > 1 && dy > 1) {
        vec2 wp01 = { wp0.x + dx/2, wp0.y };
        vec2 wp02 = { wp0.x + dx/2, wp0.y + dy/2 };
        vec2 wp03 = { wp0.x, wp0.y + dy/2 };
        vec2 wp04 = { wp0.x, wp0.y };

        vec2 wp11 = { wp1.x, wp1.y - dy/2 };
        vec2 wp12 = { wp1.x, wp1.y };
        vec2 wp13 = { wp1.x - dx/2, wp1.y };
        vec2 wp14 = { wp1.x - dx/2, wp1.y - dy/2 };

        draw_varnok(wp01, wp11, show_steps);
        draw_varnok(wp02, wp12, show_steps);
        draw_varnok(wp03, wp13, show_steps);
        draw_varnok(wp04, wp14, show_steps);
    } else if (dx > 1) {
        vec2 wp01 = { wp0.x, wp0.y };
        vec2 wp02 = { wp0.x + dx/2, wp0.y };

        vec2 wp11 = { wp1.x - dx/2, wp1.y };
        vec2 wp12 = { wp1.x, wp1.y };

        draw_varnok(wp01, wp11, show_steps);
        draw_varnok(wp02, wp12, show_steps);
    } else {
        vec2 wp01 = { wp0.x, wp0.y };
        vec2 wp02 = { wp0.x, wp0.y + dy/2 };

        vec2 wp11 = { wp1.x, wp1.y - dy/2 };
        vec2 wp12 = { wp1.x, wp1.y };

        draw_varnok(wp01, wp11, show_steps);
        draw_varnok(wp02, wp12, show_steps);
    }
}

int main(void)
{
    setlocale(LC_ALL, "Russian");

    image = & (image_t) { 0 };
    overlay_image = & (image_t) { 0 };

    wprintf(L"Введите x-координату окна: \n> ");
    window_x = parse_int();
    if (window_x < 0) error(L"x-координата не может быть отрицательной\n");
    if (window_x < 1) window_x = 1;

    wprintf(L"Введите y-координату окна: \n> ");
    window_y = parse_int();
    if (window_y < 0) error(L"y-координата не может быть отрицательной\n");
    if (window_y < 1) window_y = 1;

    wprintf(L"Введите ширину окна: \n> ");
    window_width = parse_int();
    if (window_width <= 0) error(L"Ширина должна быть положительной\n");

    wprintf(L"Введите высоту окна: \n> ");
    window_height = parse_int();
    if (window_height <= 0) error(L"Высота должна быть положительной\n");

    wprintf(L"Введите размер одной клетки в пикселях: \n> ");
    scale = parse_int();
    if (scale <= 0) error(L"Размер клетки должен быть положительным\n");

    image->w = (window_width + window_x + 1) * scale;
    image->h = (window_height + window_y + 1) * scale;
    image->data = malloc(image->w * image->h * 4);
    memset(image->data, 0xff, image->w * image->h * 4);

    overlay_image->w = image->w;
    overlay_image->h = image->h;
    overlay_image->data = malloc(image->w * image->h * 4);
    memset(overlay_image->data, 0xff, image->w * image->h * 4);

    wprintf(L"Выберите действие (введите цифру): \n"
            L"  1 - выполнить алгоритм Варнока\n"
            L"  2 - показать поэтапное выполнение алгоритма Варнока\n"
            L"  3 - выход из программы\n"
            L"> ");
    int choice = parse_int();
    if (choice == 3) exit(0);

    wprintf(L"Введите название файла, в который необходимо сохранять результат:\n> ");
    if (scanf("%1023s*c", filename) != 1) {
        error(L"Не удалось прочитать название файла\n");
    }

    polygons = & (array_t) {
        .items = (array_t[]) {
            {
                .items = (vec3[]) { {0,0,2}, {2,0,2}, {2,2,2}, {0,2,2} },
                .len = 4, .item_size = sizeof(vec3)
            },
            {
                .items = (vec3[]) { {6,0,3}, {8,0,3}, {8,2,3}, {6,2,3} },
                .len = 4, .item_size = sizeof(vec3)
            },
            {
                .items = (vec3[]) { {0,0,4}, {8,0,4}, {8,8,4}, {0,8,4} },
                .len = 4, .item_size = sizeof(vec3)
            }
        }, .len = 3, .item_size = sizeof(array_t)
    };
    colors = (u32[]) { 0xff0000ff, 0xff00ff00, 0xffff0000 };

    char c;
    while ((c = getchar()) != '\n' && c != EOF);

    draw_varnok((vec2) { 0, 0 },
                (vec2) { window_width, window_height },
                choice == 2);
    memset(overlay_image->data, 0x00, overlay_image->w * overlay_image->h * 4);
    draw_axes();
    for (int x = 0; x < overlay_image->w; x++) {
        for (int y = 0; y < overlay_image->h; y++) {
            if (image_is_color(overlay_image, x, y, 0)) {
                image_set(overlay_image, x, y, *(u32*)image_at(image, x, y));
            }
        }
    }

    image_write(filename, overlay_image);

    free(image->data);
}
