#include "Render.h"

#include <string.h>

#define MAX_DISTANCE 1000.0f

static Color Render_Vec3ToColor(vec3s color);
static Color RenderData_ComputePixel(const RenderData *data, int i, int j);

static Color Render_Vec3ToColor(vec3s color) {
	color = glms_vec3_scale(glms_vec3_clamp(color, 0.0f, 1.0f), 255.0f);

	return (Color) {
		(int) color.raw[0],
		(int) color.raw[1],
		(int) color.raw[2],
	};
}

static Color RenderData_ComputePixel(const RenderData *data, int i, int j) {
	int w = data->config.w;
	int h = data->config.h;

	vec3s dir = glms_vec3_normalize((vec3s){
		(float) (i - w / 2) / h,
		(float) -(j - h / 2) / h,
		1.0f
	});

	dir = glms_vec3_rotate(
			dir,
			data->config.yaw,
			(vec3s) {0.0f, 1.0f, 0.0f}
			);


	Line line;

	line.start = data->config.camera_position;
	line.end = glms_vec3_add(line.start, glms_vec3_scale(dir, 1000.0f));

	LineHit hit;

	hit = Scene_Trace(&data->scene, &line);

	if(!hit.found) return (Color) {0,0,0};

	vec3s found_color = Scene_GetLightContribution(&data->scene, &hit);

	return Render_Vec3ToColor(found_color);
}

void RenderData_CleanStart(RenderData *render_data) {
	Shape *shape;
	memset(render_data, 0, sizeof(RenderData));

	render_data->config.w = 2560;
	render_data->config.h = 1440;

	Image_Create(&render_data->image, render_data->config.w, render_data->config.h);

	shape = Scene_AddPlane(
			&render_data->scene,
			(Plane) {
				.normal = {0.0f, 1.0f, 0.0f},
				.d = -1.0f
			}
			);

	//shape->k_diffuse = (vec3s) {1.0f, 0.0f, 0.0f};

	shape = Scene_AddSphere(
			&render_data->scene,
			(Sphere) {
				.center = {0.0f, 0.1f, 8.0f},
				.radius = 1.0f,
			}
			);

	shape->k_diffuse = (vec3s) {0.0f, 1.0f, 1.0f};
	shape->k_specular = (vec3s) {1.0f, 1.0f, 1.0f};
	shape->shineness = 8;

	render_data->scene.ambient = (vec3s) {0.2f, 0.2f, 0.2f};

	Scene_AddLight(
			&render_data->scene,
			(Light) {
				.position = {2.0f, 8.0f, -2.0f},
				.diffuse = {1.0f, 1.0f, 1.0f},
				.specular = {1.0f, 1.0f, 1.0f},
			}
			);
}

void RenderData_Render(RenderData *data) {
	int w = data->config.w;
	int h = data->config.h;

	int i, j;

	#pragma omp parallel for collapse(2) private(i) schedule(dynamic)
	for(j = 0; j < h; j++) {
		for(i = 0; i < w; i++) {
			Image_PutPixel(
					&data->image,
					i,
					j,
					RenderData_ComputePixel(data, i, j)
					);
		}
	}

	Image_Export(&data->image, "tmp.png");
}
