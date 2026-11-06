#include "Scene.h"

#include <stb_ds.h>

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

	return closest_hit;
}
