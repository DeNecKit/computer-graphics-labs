#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "stb_image.h"
#include "stb_image_write.h"

typedef struct { float r, g ,b; } rgb_t;

typedef struct { float h, s, v; } hsv_t;

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

rgb_t hsv_to_rgb(hsv_t cin)
{
    float c = cin.v * cin.s;
    float x = c * (1.f - fabsf(fmodf(cin.h / 60.f, 2.f) - 1.f));
    float m = cin.v - c;
    rgb_t cout = { 0 };
    if (cin.h < 60) cout = (rgb_t){ c, x, 0 };
    else if (cin.h < 120) cout = (rgb_t){ x, c, 0 };
    else if (cin.h < 180) cout = (rgb_t){ 0, c, x };
    else if (cin.h < 240) cout = (rgb_t){ 0, x, c };
    else if (cin.h < 300) cout = (rgb_t){ x, 0, c };
    else cout = (rgb_t){ c, 0, x };
    cout.r += m;
    cout.g += m;
    cout.b += m;
    cout.r = fmaxf(fminf(cout.r, 1.f), 0.f);
    cout.g = fmaxf(fminf(cout.g, 1.f), 0.f);
    cout.b = fmaxf(fminf(cout.b, 1.f), 0.f);
    return cout;
}

int main(void)
{
    const char *input = "image.jpg";
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

    const char *output1 = "image_contrast.jpg";
    const char *output2 = "image_overlapped.jpg";
    stbi_write_jpg(output1, w, h, ch, image_contrast, 100);
    printf("Saved \"%s\"\n", output1);
    stbi_write_jpg(output2, w, h, ch, image_overlapped, 100);
    printf("Saved \"%s\"\n", output2);
    stbi_image_free(image);
    free(image_contrast);
    free(image_overlapped);
}
