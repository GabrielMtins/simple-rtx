#include <stdio.h>

#include "Image.h"

int main(int argc, char **argv) {
	(void) argc;
	(void) argv;

	printf("hi\n");
	Image image;

	Image_Create(&image, 64, 64);

	for(int i = 0; i < 32; i++) {
		Image_PutPixel(
				&image,
				i,
				i,
				(Color) {0xff, 0x00, 0x00}
				);
	}

	Image_Export(&image, "./tmp.png");

	return 0;
}
