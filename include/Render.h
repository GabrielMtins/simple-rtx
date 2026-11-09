#ifndef RENDER_H
#define RENDER_H

#include "Scene.h"
#include "Image.h"

#define EXPORT_FILENAME_SIZE 63
typedef struct ExportConfig {
	int w, h;
	char filename[EXPORT_FILENAME_SIZE + 1];
	int recursion_depth;
} ExportConfig;

typedef struct RenderData {
	Scene scene;
	Image image;
	ExportConfig export;
} RenderData;

void Render_CleanStart(RenderData *render_data);

Color Render_ComputePixel(const RenderData *data, int i, int j);

void Render_RenderImage(RenderData *render_data);

#endif
