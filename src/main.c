#include <stdio.h>

#include "Render.h"
#include "Image.h"

int main(int argc, char **argv) {
	(void) argc;
	(void) argv;

	RenderData render_data;

	RenderData_CleanStart(&render_data);
	RenderData_Render(&render_data);

	return 0;
}
