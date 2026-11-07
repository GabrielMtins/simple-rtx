#include "Render.h"

#include <string.h>
#include <stb_ds.h>

#define MAX_RECURSION 10

typedef HitCtx * HitCtxVector;

static Color Render_Vec3ToColor(vec3s color);
static Color Render_ComputeLineHitColor(const RenderData *data, const Line *line);
static Color Render_ComputePixel(const RenderData *data, int i, int j);

static Color Render_Vec3ToColor(vec3s color) {
	color = glms_vec3_scale(glms_vec3_clamp(color, 0.0f, 1.0f), 255.0f);

	return (Color) {
		(int) color.raw[0],
		(int) color.raw[1],
		(int) color.raw[2],
	};
}

static Color Render_ComputeLineHitColor(const RenderData *data, const Line *line) {
	vec3s final_color = GLMS_VEC3_ZERO;
	Line next_line = *line;

	HitCtxVector ctx_list = NULL;

	for(int i = 0; i < MAX_RECURSION; i++) {
		HitCtx ctx = HitCtx_Create(&next_line);

		if(!Scene_Trace(&data->scene, &ctx)) break;

		arrput(ctx_list, ctx);
		
		if(!Scene_ShouldReflect(&data->scene, &ctx)) {
			break;
		}

		next_line = Scene_GetReflectionLine(&data->scene, &ctx);
	}

	while(arrlen(ctx_list) > 0) {
		HitCtx ctx = arrpop(ctx_list);

		ctx.reflection = final_color;

		final_color = Scene_GetLightContribution(&data->scene, &ctx);
	}

	arrfree(ctx_list);

	return Render_Vec3ToColor(final_color);
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

	Line line = {
		.position = data->scene.camera.position,
		.dir = dir,
	};

	return Render_ComputeLineHitColor(data, &line);
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
