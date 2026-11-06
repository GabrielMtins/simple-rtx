#ifndef IMAGE_H
#define IMAGE_H

#include <stdint.h>

typedef struct Color {
	uint8_t r, g, b;
} Color;

typedef struct Image {
	uint8_t *data;
	int w, h;
} Image;

void Image_Create(Image *image, int w, int h);

void Image_PutPixel(Image *image, int i, int j, Color color);

int Image_Export(Image *image, const char *filename);

void Image_Free(Image *image);

#endif
