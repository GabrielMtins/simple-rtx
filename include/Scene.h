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
} Scene;

LineHit Scene_Trace(const Scene *scene, const Line *line);

#endif
