#include "Shape.h"

#include <math.h>

LineHit Shape_GetLineHit(const Shape *mesh, const Line *line) {
	LineHit hit;

	#define EXPAND_AS_CASE(id, type, hit_func) \
		case id: \
			hit = hit_func(&mesh->data._##type, line); \
		break;

	switch(mesh->type) {
		FOR_SHAPE_LIST(EXPAND_AS_CASE)

		default:
			hit.found = false;
			break;
	}
	#undef EXPAND_AS_CASE

	return hit;
}

LineHit Sphere_GetLineHit(const Sphere *sphere, const Line *line) {
	LineHit hit;

	hit.found = false;

	vec3s origin = line->start;
	vec3s origin_minus_center = glms_vec3_sub(origin, sphere->center);
	vec3s dir = LINE_GET_DIR(*line);

	float a = glms_dot(dir, dir);
	float b = glms_dot(dir, origin_minus_center);
	float c = glms_dot(origin_minus_center, origin_minus_center) - sphere->radius * sphere->radius;

	float delta = b * b - 4 * a * c;

	if(delta < 0) return hit;

	float t1, t2;

	float sqrt_delta = sqrtf(delta);

	t1 = (-b - sqrt_delta) / (2 * a);
	t2 = (-b + sqrt_delta) / (2 * a);

	if(t1 < t2 && t1 > 0.0f && t1 < 1.0f) {
		hit.distance = t1;
	} else if (t2 < t1 && t2 > 0.0f && t2 < 1.0f) {
		hit.distance = t2;
	} else {
		return hit;
	}

	hit.found = true;
	hit.normal = glms_normalize(glms_vec3_sub(
			glms_vec3_add(origin, glms_vec3_scale(dir, hit.distance)),
			sphere->center
			));

	return hit;
}

LineHit Plane_GetLineHit(const Plane *plane, const Line *line) {
	LineHit hit;

	hit.found = false;

	float numerator = (plane->d - glms_dot(plane->normal, line->start));
	float denominator = glms_dot(plane->normal, LINE_GET_DIR(*line));

	if(fabsf(denominator) < 0.01f) {
		return hit;
	}

	hit.distance = numerator / denominator;

	if(hit.distance < 0.0f || hit.distance > 1.0f) {
		return hit;
	}

	hit.found = true;
	hit.normal = plane->normal;

	return hit;
}
