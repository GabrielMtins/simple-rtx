#include "Scene.h"

#include <stb_ds.h>

static bool Light_IsVisibleFromHit(const Scene *scene, const LineHit *hit, const Light *light);
static vec3s Light_GetShapeAmbientColor(const Scene *scene, const HitCtx *ctx);
static float Light_GetAttenuation(const LineHit *hit, const Light *light);
static vec3s Light_GetContribution(const Scene *scene, const HitCtx *ctx, int light_index);
static vec3s Light_GetReflectionLight(const Scene *scene, const HitCtx *ctx);

static bool Light_IsVisibleFromHit(const Scene *scene, const LineHit *hit, const Light *light) {
	HitCtx ctx = {
		.line = {
			.position = hit->position,
			.dir = glms_vec3_normalize(glms_vec3_sub(light->position, hit->position)),
		}
	};

	ctx.line.position = LINE_GET_POINT(ctx.line, 0.1f);

	if(!Scene_Trace(scene, &ctx)) return true;

	return ctx.hit.distance > glms_vec3_distance(light->position, hit->position);
}

static vec3s Light_GetShapeAmbientColor(const Scene *scene, const HitCtx *ctx) {
	Shape *shape = &scene->shapes[ctx->hit.shape_index];

	return glms_vec3_mul(
			scene->ambient,
			shape->diffuse
			);
}

static float Light_GetAttenuation(const LineHit *hit, const Light *light) {
	if(fabsf(light->distance) < 0.01f) {
		return 0.0f;
	}

	float k =
		glms_vec3_distance(hit->position, light->position) /
		light->distance;

	k = glm_clamp(k, 0.0f, 1.0f);

	k = 1.0f - k;

	k *= k;

	return k;
}

static vec3s Light_GetContribution(const Scene *scene, const HitCtx *ctx, int light_index) {
	vec3s final_color = glms_vec3_zero();

	const LineHit *hit = &ctx->hit;
	const Light *light = &scene->lights[light_index];
	const Shape *shape = &scene->shapes[hit->shape_index];

	if(!Light_IsVisibleFromHit(scene, hit, light)) {
		return final_color;
	}

	vec3s light_dir = glms_normalize(glms_vec3_sub(light->position, hit->position));
	vec3s view_dir = glms_normalize(glms_vec3_sub(scene->camera.position, hit->position));
	vec3s ref_dir = glms_normalize(
			glms_vec3_sub(
				glms_vec3_scale(
					hit->normal,
					2.0f * glms_dot(light_dir, hit->normal)
					),
				light_dir
				)
			);

	float diffuse_part = glms_dot(light_dir, hit->normal);
	float ref_part = glms_dot(ref_dir, view_dir);

	if(diffuse_part > 0) {
		final_color = glms_vec3_add(
				final_color,
				glms_vec3_scale(
					glms_vec3_mul(shape->diffuse, light->diffuse),
					diffuse_part
					)
				);
	}

	if(ref_part > 0) {
		ref_part = powf(ref_part, shape->shineness);

		final_color = glms_vec3_add(
				final_color,
				glms_vec3_scale(
					glms_vec3_mul(shape->specular, light->specular),
					ref_part
					)
				);
	}

	return glms_vec3_scale(
			final_color,
			Light_GetAttenuation(hit, light)
			)
			;
}

static vec3s Light_GetReflectionLight(const Scene *scene, const HitCtx *ctx) {
	const Shape *shape = &scene->shapes[ctx->hit.shape_index];

	return glms_vec3_scale(ctx->reflection, shape->part_reflection);
}

HitCtx HitCtx_Create(const Line *line) {
	return (HitCtx) {
		.line = *line,
		.reflection = GLMS_VEC3_ZERO,
	};
}

bool Scene_Trace(const Scene *scene, HitCtx *ctx) {
	int len = arrlen(scene->shapes);

	ctx->hit.found = false;

	for(int i = 0; i < len; i++) {
		LineHit current_hit;

		current_hit = Shape_GetLineHit(
				&scene->shapes[i],
				&ctx->line
				);

		if(!current_hit.found) continue;

		if(!ctx->hit.found || current_hit.distance < ctx->hit.distance) {
			ctx->hit = current_hit;
			ctx->hit.shape_index = i;
		}
	}

	return ctx->hit.found;
}

bool Scene_ShouldReflect(const Scene *scene, const HitCtx *ctx) {
	if(!ctx->hit.found) return false;

	const Shape *shape = &scene->shapes[ctx->hit.shape_index];

	if(shape->part_reflection < 0.01f) return false;

	return true;
}

Line Scene_GetReflectionLine(const Scene *scene, const HitCtx *ctx) {
	(void) scene;

	vec3s ref = glms_normalize(
			glms_vec3_sub(
				glms_vec3_scale(
					ctx->hit.normal,
					2.0f * glms_dot(ctx->line.dir, ctx->hit.normal)
					),
				ctx->line.dir
				)
			);

	Line line = {
		.position = ctx->hit.position,
		.dir = glms_vec3_scale(ref, -1.0f),
	};

	line.position = LINE_GET_POINT(line, 0.01f);

	return line;
}

vec3s Scene_GetLightContribution(const Scene *scene, const HitCtx *ctx) {
	int len;
	vec3s final_color = Light_GetShapeAmbientColor(scene, ctx);

	len = arrlen(scene->lights);

	for(int i = 0; i < len; i++) {
		vec3s current_color;

		current_color = Light_GetContribution(scene, ctx, i);

		final_color = glms_vec3_add(current_color, final_color);
	}

	final_color = glms_vec3_add(
			final_color,
			Light_GetReflectionLight(scene, ctx)
			);

	return final_color;
}

void Scene_AddLight(Scene *scene, Light light) {
	arrput(scene->lights, light);
}

Shape * Scene_AddShape(Scene *scene, ShapeType type) {
	Shape *shape = arraddnptr(scene->shapes, 1);

	memset(shape, 0, sizeof(Shape));

	shape->type = type;
	shape->shineness = 8.0f;
	shape->diffuse = (vec3s) {1.0f, 1.0f, 1.0f};
	shape->part_reflection = 0.0f;

	return shape;
}
