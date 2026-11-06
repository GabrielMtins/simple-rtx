#ifndef RENDER_H
#define RENDER_H

#include "Scene.h"
#include "Image.h"

typedef struct SceneConfig {
	int w, h;
	vec3s camera_position;
	float pitch, yaw;
	char *filename;
} SceneConfig;

typedef struct RenderData {
	Scene scene;
	Image image;
	SceneConfig config;
} RenderData;

void RenderData_CleanStart(RenderData *render_data);

void RenderData_Render(RenderData *render_data);

#endif
