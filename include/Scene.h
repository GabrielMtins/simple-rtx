#ifndef SCENE_H
#define SCENE_H

#include "Shape.h"

typedef struct Light {
	vec3s position;

	vec3s diffuse;
	vec3s specular;

	float distance;
} Light;

typedef Shape * ShapeVector;
typedef Light * LightVector;

typedef struct Scene {
	ShapeVector shapes;
	LightVector lights;

	struct {
		vec3s position;
		float pitch;
		float yaw;
	} camera;

	vec3s ambient;
} Scene;

typedef struct HitCtx {
	Line line;
	LineHit hit;
	vec3s reflection;
} HitCtx;

bool Scene_Trace(const Scene *scene, HitCtx *ctx);

Line Scene_GetReflectionLine(const Scene *scene, const HitCtx *ctx);

vec3s Scene_GetLightContribution(const Scene *scene, HitCtx *ctx);

void Scene_AddLight(Scene *scene, Light light);

Shape * Scene_AddShape(Scene *scene, ShapeType type);

#endif
