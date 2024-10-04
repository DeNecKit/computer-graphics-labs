#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "stb_image.h"
#include "stb_image_write.h"

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

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Expected image filename\n");
        exit(1);
    }
    if (argc < 5) {
        fprintf(stderr, "Expected 3 coefficients for affine transformations\n");
        exit(1);
    }
    if (argc < 8) {
        fprintf(stderr, "Expected 3 output filenames\n");
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

    const float phi = atof(argv[2]) * M_PI / 180;
    const float scale_x = atof(argv[3]);
    const float scale_y = atof(argv[4]);

    const char *output1 = argv[5];
    const char *output2 = argv[6];
    const char *output3 = argv[7];

    unsigned char *image_output1 = malloc(size);
    unsigned char *image_output2 = malloc(size);
    unsigned char *image_output3 = malloc(size);
    memset(image_output1, 0x00, size);
    memset(image_output2, 0x00, size);
    memset(image_output3, 0x00, size);

    for (int x = 0; x < w; x++) {
        for (int y = 0; y < h; y++) {
            int cx = ((x-w/2)*cosf(phi) - (y-h/2)*sinf(phi)) * (1/scale_x) + w/2;
            int cy = ((x-w/2)*sinf(phi) + (y-h/2)*cosf(phi)) * (1/scale_y) + h/2;
            if (cx < 0 || cy < 0 || cx >= w || cy >= h) continue;
            int i = (y*w + x) * ch;
            int j = (cy*w + cx) * ch;
            image_output1[i]     = image[j];
            image_output1[i + 1] = image[j + 1];
            image_output1[i + 2] = image[j + 2];
        }
    }

    for (int x = 0; x < w; x++) {
        for (int y = 0; y < h; y++) {
            int sx = (x-w/2) * scale_x + w/2;
            int sy = (y-h/2) * scale_y + h/2;
            int cx = ((sx-w/2)*cosf(-phi) - (sy-h/2)*sinf(-phi)) + w/2;
            int cy = ((sx-w/2)*sinf(-phi) + (sy-h/2)*cosf(-phi)) + h/2;
            if (cx < 0 || cy < 0 || cx >= w || cy >= h) continue;
            int i = (y*w + x) * ch;
            int j = (cy*w + cx) * ch;
            image_output2[i]     = image_output1[j];
            image_output2[i + 1] = image_output1[j + 1];
            image_output2[i + 2] = image_output1[j + 2];
        }
    }

    for (int x = 0; x < w; x++) {
        for (int y = 0; y < h; y++) {
            int cx = sinhf(x);
            int cy = y;
            if (cx < 0 || cx >= w) continue;
            int i = (y*w + x) * ch;
            int j = (cy*w + cx) * ch;
            image_output3[i]     = image[j];
            image_output3[i + 1] = image[j + 1];
            image_output3[i + 2] = image[j + 2];
        }
    }
    
    write_image(output1, w, h, ch, image_output1);
    write_image(output2, w, h, ch, image_output2);
    write_image(output3, w, h, ch, image_output3);

    stbi_image_free(image);
    free(image_output1);
    free(image_output2);
    free(image_output3);
}
