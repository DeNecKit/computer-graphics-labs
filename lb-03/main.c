#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "stb_image.h"
#include "stb_image_write.h"

typedef struct { float r, g ,b; } rgb_t;
typedef struct { float h, s, v; } hsv_t;

const char *get_file_ext(const char *filename)
{
    while (*filename && *filename != '.') filename++;
    return filename;
}

void write_image(const char *filename,
                 int w, int h, int ch,
                 unsigned char *data)
{
    const char *ext = get_file_ext(filename);
    if (strcmp(ext, ".jpg") == 0) {
        stbi_write_jpg(filename, w, h, ch, data, 100);
    } else if (strcmp(ext, ".png") == 0) {
        stbi_write_png(filename, w, h, ch, data, w * ch);
    } else {
        fprintf(stderr, "Unsupported file extension: \"%s\"\n", ext);
        exit(1);
    }
}

hsv_t rgb_to_hsv(rgb_t cin)
{
    hsv_t cout = { 0 };
    float min = fminf(fminf(cin.r, cin.g), cin.b);
    float max = fmaxf(fmaxf(cin.r, cin.g), cin.b);
    cout.v = max;
    float delta = max - min;
    cout.h = delta < 0.002f
        ? 0.f : max == cin.r
        ? fmodf((cin.g - cin.b) / delta, 6.f) : max == cin.g
        ? (cin.b - cin.r) / delta + 2.f
        : (cin.r - cin.g) / delta + 4.f;
    cout.h *= 60.f;
    cout.s = max == 0
        ? 0.f
        : delta / max;
    return cout;
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Expected image filename\n");
        exit(1);
    }
    if (argc < 4) {
        fprintf(stderr, "Expected 2 output image filenames\n");
        exit(1);
    }
    const char *input = argv[1];
    const char *output1 = argv[2];
    const char *output2 = argv[3];
    int w, h, ch;
    unsigned char *image = stbi_load(input, &w, &h, &ch, STBI_rgb);
    const int n = w * h;
    const int size = n * ch;
    unsigned char *image_contrast   = malloc(size);
    unsigned char *image_overlapped = malloc(size);
    
    for (int i = 0; i < n; i++) {
        rgb_t rgb = {
            image[ch*i] / 255.f,
            image[ch*i + 1] / 255.f,
            image[ch*i + 2] / 255.f
        };
        rgb_t rgb_orig = rgb;
        hsv_t hsv = rgb_to_hsv(rgb);
        const float alpha = 1.5f;
        if (hsv.v > 0.2f) {
            rgb.r = fmaxf(fminf((rgb.r - 0.5f) * alpha + 0.5f, 1.f), 0.f);
            rgb.g = fmaxf(fminf((rgb.g - 0.5f) * alpha + 0.5f, 1.f), 0.f);
            rgb.b = fmaxf(fminf((rgb.b - 0.5f) * alpha + 0.5f, 1.f), 0.f);
        }
        image_contrast[ch*i]     = rgb.r * 255.f;
        image_contrast[ch*i + 1] = rgb.g * 255.f;
        image_contrast[ch*i + 2] = rgb.b * 255.f;

        image_overlapped[ch*i]     = fmaxf(fminf(rgb_orig.r * rgb.r, 1.f), 0.f) * 255.f;
        image_overlapped[ch*i + 1] = fmaxf(fminf(rgb_orig.g * rgb.g, 1.f), 0.f) * 255.f;
        image_overlapped[ch*i + 2] = fmaxf(fminf(rgb_orig.b * rgb.b, 1.f), 0.f) * 255.f;
    }

    write_image(output1, w, h, ch, image_contrast);
    printf("Saved \"%s\"\n", output1);
    write_image(output2, w, h, ch, image_overlapped);
    printf("Saved \"%s\"\n", output2);
    stbi_image_free(image);
    free(image_contrast);
    free(image_overlapped);
}
