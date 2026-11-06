#ifndef SCENE_H
#define SCENE_H

#include "Shape.h"

typedef struct Light {
	vec3s position;

	vec3s diffuse;
	vec3s specular;

	float attenuation;
} Light;

typedef Shape * ShapeVector;
typedef Light * LightVector;

typedef struct Scene {
	ShapeVector shapes;
	LightVector lights;

	vec3s camera_position;
	vec3s ambient;
} Scene;

LineHit Scene_Trace(const Scene *scene, const Line *line);

vec3s Scene_GetLightContribution(const Scene *scene, const LineHit *hit);

void Scene_AddLight(Scene *scene, Light light);

#define EXPAND_AS_FUNC(id, type, ...) Shape * Scene_Add##type(Scene *scene, type _##type);
FOR_SHAPE_LIST(EXPAND_AS_FUNC)
#undef EXPAND_AS_FUNC

#endif
