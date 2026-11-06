#include "Scene.h"

#include <stb_ds.h>

static bool Light_IsVisibleFromHit(const Scene *scene, const LineHit *hit, const Light *light);
static vec3s Light_GetShapeAmbientColor(const Scene *scene, const LineHit *hit);
static float Light_GetAttenuation(const LineHit *hit, const Light *light);
static vec3s Light_GetContribution(const Scene *scene, const LineHit *hit, int light_index);

static bool Light_IsVisibleFromHit(const Scene *scene, const LineHit *hit, const Light *light) {
	bool visible;

	Line line = {
		.start = hit->position,
		.end = light->position
	};

	line.start = glms_vec3_lerp(line.start, line.end, 0.01f);

	visible = !Scene_Trace(scene, &line).found;

	return visible;
}

static vec3s Light_GetShapeAmbientColor(const Scene *scene, const LineHit *hit) {
	Shape *shape = &scene->shapes[hit->shape_index];

	return glms_vec3_mul(scene->ambient, shape->k_diffuse);
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

static vec3s Light_GetContribution(const Scene *scene, const LineHit *hit, int light_index) {
	vec3s final_color = glms_vec3_zero();

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
					glms_vec3_mul(shape->k_diffuse, light->diffuse),
					diffuse_part
					)
				);
	}

	if(ref_part > 0) {
		ref_part = powf(ref_part, shape->shineness);

		final_color = glms_vec3_add(
				final_color,
				glms_vec3_scale(
					glms_vec3_mul(shape->k_specular, light->specular),
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

LineHit Scene_Trace(const Scene *scene, const Line *line) {
	LineHit closest_hit;
	int len = arrlen(scene->shapes);

	closest_hit.found = false;

	for(int i = 0; i < len; i++) {
		LineHit current_hit;

		current_hit = Shape_GetLineHit(
				&scene->shapes[i],
				line
				);

		if(!current_hit.found) continue;

		if(!closest_hit.found || current_hit.distance < closest_hit.distance) {
			closest_hit = current_hit;
			closest_hit.shape_index = i;
		}
	}

	if(closest_hit.found) {
		closest_hit.position = glms_vec3_lerp(line->start, line->end, closest_hit.distance);
	}

	return closest_hit;
}

vec3s Scene_GetLightContribution(const Scene *scene, const LineHit *hit) {
	int len;
	vec3s final_color = Light_GetShapeAmbientColor(scene, hit);

	len = arrlen(scene->lights);

	for(int i = 0; i < len; i++) {
		vec3s current_color;

		current_color = Light_GetContribution(scene, hit, i);

		final_color = glms_vec3_add(current_color, final_color);
	}

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
	shape->k_diffuse = (vec3s) {1.0f, 1.0f, 1.0f};

	return shape;
}

#define EXPAND_AS_FUNC(id, dtype, ...) \
	Shape * Scene_Add##dtype(Scene *scene, dtype _##dtype) { \
		Shape shape = { \
			.type = id, \
			.data._##dtype = _##dtype, \
			.k_diffuse = {1.0f, 1.0f, 1.0f}, \
			.k_specular = {0.0f, 0.0f, 0.0f}, \
		}; \
		arrput(scene->shapes, shape); \
		return &scene->shapes[arrlen(scene->shapes) - 1];\
	}
FOR_SHAPE_LIST(EXPAND_AS_FUNC)
#undef EXPAND_AS_FUNC
