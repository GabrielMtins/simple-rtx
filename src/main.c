#include <stdio.h>
#include <stdlib.h>

#include "Parser.h"
#include "Render.h"
#include "Image.h"

int main(int argc, char **argv) {
	(void) argc;
	(void) argv;

	/*
	FILE *file;

	file = fopen("teste.scene", "rb");

	fseek(file, 0, SEEK_END);
	int size = ftell(file);

	fseek(file, 0, SEEK_SET);

	char *tmp = malloc(size + 1);

	fread(tmp, 1, size, file);

	fclose(file);

	tmp[size] = '\0';
	*/

	/*
	Parser parser = Parser_Create(tmp, Test_Callback, NULL);

	Parser_ParseData(&parser);
	*/

	RenderData render_data;

	Render_CleanStart(&render_data);

	Parser_ParseFile(&render_data, "scene.json");

	Render_RenderImage(&render_data);


	/*
	RenderData_Render(&render_data);
	*/

	return 0;
}
