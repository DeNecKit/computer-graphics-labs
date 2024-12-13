#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "stb_image.h"
#include "stb_image_write.h"

#define PI 3.141592653589793

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
    printf("Wrote to \"%s\"\n", filename);
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "Expected image filename\n");
        exit(1);
    }
    if (argc < 3) {
        fprintf(stderr, "Expected output image filename\n");
        exit(1);
    }
    const char *input = argv[1];
    int w, h, ch;
    unsigned char *image = stbi_load(input, &w, &h, &ch, STBI_rgb);
    if (image == NULL) {
        fprintf(stderr, "Couldn't open file \"%s\"\n", input);
        exit(1);
    }
    const int n = w * h;
    const int size = n * ch;

    const float phi = 45.f * PI / 180.f;
    const float scale_x = 1.5f;
    const float scale_y = 1.5f;

    const char *output = argv[2];

    unsigned char *image_output = malloc(size);
    memset(image_output, 0x00, size);

    for (int x = 0; x < w; x++) {
        for (int y = 0; y < h; y++) {
            float cx = ((x-w/2)*cosf(phi) - (y-h/2)*sinf(phi)) * (1/scale_x) + w/2;
            cx += 0.5f*(y-h/2);
            float cy = ((x-w/2)*sinf(phi) + (y-h/2)*cosf(phi)) * (1/scale_y) + h/2;
            int l = (int)cx;
            int k = (int)cy;
            if (l < 0 || k < 0 || l >= w || k >= h) continue;
            float a = cx - l;
            float b = cy - k;
            int i = (y*w + x) * ch;

            float clr_r = 0.f;
            float clr_g = 0.f;
            float clr_b = 0.f;
            int j = (k*w + l) * ch;
            clr_r += image[j]*(1-a)*(1-b);
            clr_g += image[j+1]*(1-a)*(1-b);
            clr_b += image[j+2]*(1-a)*(1-b);
            if (l+1 < w) {
                j = (k*w + l+1) * ch;
                clr_r += image[j]*a*(1-b);
                clr_g += image[j+1]*a*(1-b);
                clr_b += image[j+2]*a*(1-b);
            }
            if (k+1 < h) {
                j = ((k+1)*w + l) * ch;
                clr_r += image[j]*b*(1-a);
                clr_g += image[j+1]*b*(1-a);
                clr_b += image[j+2]*b*(1-a);
            }
            if (l+1 < w && k+1 < h) {
                j = ((k+1)*w + l+1) * ch;
                clr_r += image[j]*a*b;
                clr_g += image[j+1]*a*b;
                clr_b += image[j+2]*a*b;
            }

            image_output[i]   = (int)clr_r;
            image_output[i+1] = (int)clr_g;
            image_output[i+2] = (int)clr_b;
        }
    }
    
    write_image(output, w, h, ch, image_output);

    stbi_image_free(image);
    free(image_output);
}
