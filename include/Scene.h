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

LineHit Scene_Trace(const Scene *scene, const Line *line);

vec3s Scene_GetLightContribution(const Scene *scene, const LineHit *hit);

void Scene_AddLight(Scene *scene, Light light);

Shape * Scene_AddShape(Scene *scene, ShapeType type);

#endif
