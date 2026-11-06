#include "Image.h"

#include <stdlib.h>
#include <string.h>

#include <stb_ds.h>
#include <stb_image_write.h>

#define NUM_CHANNELS (sizeof(Color) / sizeof(uint8_t))

void Image_Create(Image *image, int w, int h) {
	arrfree(image->data);

	arraddnindex(image->data, w * h * NUM_CHANNELS);
	image->w = w;
	image->h = h;

	memset(image->data, 0, w * h * NUM_CHANNELS);
}

void Image_PutPixel(Image *image, int i, int j, Color color) {
	if(i < 0 || j < 0 || i >= image->w || j >= image->h) return;

	int start_index = (i + j * image->w) * NUM_CHANNELS;

	image->data[start_index + 0] = color.r;
	image->data[start_index + 1] = color.g;
	image->data[start_index + 2] = color.b;
}

int Image_Export(Image *image, const char *filename) {
	return stbi_write_png(
			filename,
			image->w,
			image->h,
			NUM_CHANNELS,
			image->data,
			image->w * NUM_CHANNELS
			);
}

void Image_Free(Image *image) {
	arrfree(image->data);
}
