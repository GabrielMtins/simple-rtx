#ifndef SHAPE_H
#define SHAPE_H

#include <stdbool.h>
#include <cglm/struct.h>

typedef struct Line {
	vec3s start;
	vec3s end;
} Line;

#define LINE_GET_DIR(line) (glms_vec3_sub((line).end, (line).start))

typedef struct LineHit {
	bool found;
	vec3s normal;
	vec3s position;
	float distance;
	int shape_index;
} LineHit;

typedef struct Sphere {
	vec3s center;
	float radius;
} Sphere;

typedef struct Plane {
	/* A point p is on a plane if dot(p, normal) = d */
	vec3s normal;
	float d;
} Plane;

#define FOR_SHAPE_LIST(DO) \
	DO(SHAPE_SPHERE, Sphere, Sphere_GetLineHit) \
	DO(SHAPE_PLANE,  Plane,  Plane_GetLineHit)

#define EXPAND_AS_ENUM(id, ...) id,
typedef enum ShapeType {
	FOR_SHAPE_LIST(EXPAND_AS_ENUM)
	NUM_MESH_TYPES
} ShapeType;
#undef EXPAND_AS_ENUM

typedef struct Shape {
	ShapeType type;

	#define EXPAND_AS_DECLARATION(id, type, ...) type _##type;
	union ShapeData {
		FOR_SHAPE_LIST(EXPAND_AS_DECLARATION)
	} data;
	#undef EXPAND_AS_DECLARATION

	vec3s k_diffuse;
	vec3s k_specular;
	float shineness;
} Shape;

LineHit Shape_GetLineHit(const Shape *shape, const Line *line);

#define EXPAND_AS_FUNCTION_DECLARATION(id, type, hit_func) LineHit hit_func(const type *data, const Line *line);
FOR_SHAPE_LIST(EXPAND_AS_FUNCTION_DECLARATION)
#undef EXPAND_AS_FUNCTION_DECLARATION

#endif
