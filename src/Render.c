#include "Render.h"

#include <string.h>

#define MAX_DISTANCE 1000.0f

static Color Render_Vec3ToColor(vec3s color);
static Color Render_ComputePixel(const RenderData *data, int i, int j);

static Color Render_Vec3ToColor(vec3s color) {
	color = glms_vec3_scale(glms_vec3_clamp(color, 0.0f, 1.0f), 255.0f);

	return (Color) {
		(int) color.raw[0],
		(int) color.raw[1],
		(int) color.raw[2],
	};
}

static Color Render_ComputePixel(const RenderData *data, int i, int j) {
	int w = data->export.w;
	int h = data->export.h;

	vec3s dir = glms_vec3_normalize((vec3s){
		(float) (i - w / 2) / h,
		(float) -(j - h / 2) / h,
		1.0f
	});

	dir = glms_vec3_rotate(
			dir,
			data->scene.camera.pitch,
			(vec3s) {1.0f, 0.0f, 0.0f}
			);

	dir = glms_vec3_rotate(
			dir,
			data->scene.camera.yaw,
			(vec3s) {0.0f, 1.0f, 0.0f}
			);

	Line line;

	line.start = data->scene.camera.position;
	line.end = glms_vec3_add(line.start, glms_vec3_scale(dir, 1000.0f));

	LineHit hit;

	hit = Scene_Trace(&data->scene, &line);

	if(!hit.found) return (Color) {0,0,0};

	vec3s found_color = Scene_GetLightContribution(&data->scene, &hit);

	return Render_Vec3ToColor(found_color);
}

void Render_CleanStart(RenderData *render_data) {
	memset(render_data, 0, sizeof(RenderData));

	render_data->export.w = 640;
	render_data->export.h = 360;
}

void Render_RenderImage(RenderData *data) {
	int w = data->export.w;
	int h = data->export.h;

	int i, j;

	Image_Create(&data->image, w, h);

	#pragma omp parallel for collapse(2) private(i) schedule(dynamic)
	for(j = 0; j < h; j++) {
		for(i = 0; i < w; i++) {
			Image_PutPixel(
					&data->image,
					i,
					j,
					Render_ComputePixel(data, i, j)
					);
		}
	}

	Image_Export(&data->image, "tmp.png");
}
