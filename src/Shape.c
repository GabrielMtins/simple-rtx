#include "Shape.h"

#include <math.h>
#include <string.h>

static bool Box_IsPointInside(vec3s box_size, vec3s point, int skip_index);
static LineHit Box_FoundHitNormalized(vec3s box_size, const Line *line);

static bool Box_IsPointInside(vec3s box_size, vec3s point, int skip_index) {
	vec3s box_min = glms_vec3_scale(box_size, -0.5f);
	vec3s box_max = glms_vec3_scale(box_size, 0.5f);

	for(int i = 0; i < 3; i++) {
		if(i == skip_index) continue;

		if(point.raw[i] <= box_min.raw[i]) {
			return false;
		}

		if(point.raw[i] >= box_max.raw[i]) {
			return false;
		}
	}

	return true;
}

static LineHit Box_FoundHitNormalized(vec3s box_size, const Line *line) {
	LineHit closest_hit = {.found = false};

	vec3s box_min = glms_vec3_scale(box_size, -0.5f);
	vec3s box_max = glms_vec3_scale(box_size, +0.5f);

	static const vec3s box_normals[] = {
		{-1.0f, 0.0f, 0.0f},
		{0.0f, -1.0f, 0.0f},
		{0.0f, 0.0f, -1.0f},
		{1.0f, 0.0f, 0.0f},
		{0.0f, 1.0f, 0.0f},
		{0.0f, 0.0f, 1.0f},
	};

	for(int i = 0; i < 3; i++) {
		LineHit hit1, hit2;

		Plane p1 = {.normal = box_normals[i], .d = glms_dot(box_normals[i], box_min)};
		Plane p2 = {.normal = box_normals[i + 3], .d = glms_dot(box_normals[i + 3], box_max)};

		hit1 = Plane_GetLineHit(&p1, line);
		hit2 = Plane_GetLineHit(&p2, line);

		if(hit1.found && Box_IsPointInside(box_size, hit1.position, i)) {
			if(!closest_hit.found || hit1.distance < closest_hit.distance) {
				closest_hit = hit1;
			}
		}

		if(hit2.found && Box_IsPointInside(box_size, hit2.position, i)) {
			if(!closest_hit.found || hit2.distance < closest_hit.distance) {
				closest_hit = hit2;
			}
		}
	}

	return closest_hit;
}

LineHit Shape_GetLineHit(const Shape *shape, const Line *line) {
	LineHit hit;

	#define EXPAND_AS_CASE(id, type, hit_func) \
		case id: \
			hit = hit_func(&shape->data._##type, line); \
		break;

	switch(shape->type) {
		FOR_SHAPE_LIST(EXPAND_AS_CASE)

		default:
			hit.found = false;
			break;
	}
	#undef EXPAND_AS_CASE

	return hit;
}

ShapeType ShapeType_GetTypeFromString(const char *str) {
	ShapeType type = SHAPE_NONE;

	if(str[0] == '\0') return type;

	#define EXPAND_AS_STRING(id, type, ...) #type,
	static const char *type_str_list[] = {
		FOR_SHAPE_LIST(EXPAND_AS_STRING)
	};
	#undef EXPAND_AS_STRING

	for(int i = 0; i < NUM_SHAPE_TYPES; i++) {
		if(strcmp(str, type_str_list[i]) == 0) {
			return (ShapeType) i;
		}
	}

	return type;
}

mat3s Shape_MakeRotMat(vec3s rotations) {
	mat4s m = GLMS_MAT4_IDENTITY;

	m = glms_rotate_x(m, glm_rad(rotations.raw[0]));
	m = glms_rotate_y(m, glm_rad(rotations.raw[1]));
	m = glms_rotate_z(m, glm_rad(rotations.raw[2]));

	return glms_mat4_pick3(m);
}

LineHit Sphere_GetLineHit(const Sphere *sphere, const Line *line) {
	LineHit hit;

	hit.found = false;

	vec3s origin = line->start;
	vec3s origin_minus_center = glms_vec3_sub(origin, sphere->center);
	vec3s dir = LINE_GET_DIR(*line);

	float a = glms_dot(dir, dir);
	float b = 2 * glms_dot(dir, origin_minus_center);
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
	hit.position = glms_vec3_lerp(line->start, line->end, hit.distance);
	hit.normal = glms_normalize(glms_vec3_sub(
				hit.position,
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
	hit.position = glms_vec3_lerp(line->start, line->end, hit.distance);

	return hit;
}

LineHit Box_GetLineHit(const Box *box, const Line *line) {
	LineHit hit;

	Line transformed_line = {
		.start = glms_mat3_mulv(box->inv_rotation_matrix, glms_vec3_sub(line->start, box->center)),
		.end = glms_mat3_mulv(box->inv_rotation_matrix, glms_vec3_sub(line->end, box->center)),
	};

	hit = Box_FoundHitNormalized(box->size, &transformed_line);

	if(hit.found) {
		hit.normal = glms_mat3_mulv(box->rotation_matrix, hit.normal);
		hit.position = glms_vec3_lerp(line->start, line->end, hit.distance);
	}

	return hit;
}
