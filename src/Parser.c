#include "Parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define Parser_GetGeneric(expr, json, str, check) \
	do { \
		const cJSON *tmp = cJSON_GetObjectItemCaseSensitive(json, str); \
		if(check(tmp)) { \
			expr; \
		} \
	} while(0)

#define PARSER_GETSTR_EXPR(data) \
	do { \
		if(tmp->valuestring) { \
			strncpy(data, tmp->valuestring, sizeof(data) - 1); \
			data[strlen(data)] = '\0'; \
		} \
	} while(0)

#define PARSER_GETVEC3_EXPR(data) \
	do { \
		const cJSON *item; \
		int index = 0; \
		cJSON_ArrayForEach(item, tmp) { \
			if(index == 3) break; \
			if(!cJSON_IsNumber(item)) break; \
			data.raw[index] = item->valuedouble; \
			index++; \
		} \
	} while(0)

#define Parser_GetString(data, json, str) Parser_GetGeneric(PARSER_GETSTR_EXPR(data), json, str, cJSON_IsString)
#define Parser_GetVec3(data, json, str) Parser_GetGeneric(PARSER_GETVEC3_EXPR(data), json, str, cJSON_IsArray)
#define Parser_GetInt(data, json, str) Parser_GetGeneric(data = tmp->valueint, json, str, cJSON_IsNumber)
#define Parser_GetFloat(data, json, str) Parser_GetGeneric(data = tmp->valuedouble, json, str, cJSON_IsNumber)

static char * Parser_GetFileStr(const char *filename);
static void Parser_ParseExport(RenderData *data, const cJSON *json);
static void Parser_ParseCamera(RenderData *data, const cJSON *json);
static void Parser_ParseLights(RenderData *data, const cJSON *json);
static void Parser_ParseShapes(RenderData *data, const cJSON *json);
static void Parser_ParseShape(RenderData *data, const cJSON *json);

static char * Parser_GetFileStr(const char *filename) {
	FILE *file;
	size_t len;
	char *str;

	file = fopen(filename, "rb");

	if(file == NULL) return NULL;

	fseek(file, 0, SEEK_END);

	len = ftell(file);

	fseek(file, 0, SEEK_SET);

	str = malloc(len + 1);

	fread(str, 1, len, file);

	str[len] = '\0';

	return str;
}

static void Parser_ParseExport(RenderData *data, const cJSON *json) {
	if(!cJSON_IsObject(json)) return;

	Parser_GetInt(data->export.w, json, "width");
	Parser_GetInt(data->export.h, json, "height");
	Parser_GetString(data->export.filename, json, "file");
}

static void Parser_ParseCamera(RenderData *data, const cJSON *json) {
	if(!cJSON_IsObject(json)) return;

	Parser_GetFloat(data->scene.camera.pitch, json, "pitch");
	Parser_GetFloat(data->scene.camera.yaw, json, "yaw");
	Parser_GetVec3(data->scene.camera.position, json, "position");
	Parser_GetVec3(data->scene.ambient, json, "ambient");

	data->scene.camera.pitch = glm_rad(data->scene.camera.pitch);
	data->scene.camera.yaw = glm_rad(data->scene.camera.yaw);
}

static void Parser_ParseLights(RenderData *data, const cJSON *json) {
	if(!cJSON_IsArray(json)) return;

	const cJSON *item;

	cJSON_ArrayForEach(item, json) {
		Light light = {0};

		Parser_GetVec3(light.position, item, "position");
		Parser_GetVec3(light.diffuse, item, "diffuse");
		Parser_GetVec3(light.specular, item, "specular");
		Parser_GetFloat(light.distance, item, "distance");

		Scene_AddLight(&data->scene, light);
	}
}

static void Parser_ParseShapes(RenderData *data, const cJSON *json) {
	if(!cJSON_IsArray(json)) return;

	const cJSON *item;

	cJSON_ArrayForEach(item, json) {
		Parser_ParseShape(data, item);
	}
}

static void Parser_ParseShape(RenderData *data, const cJSON *json) {
	if(!cJSON_IsObject(json)) return;

	char type_str[32] = {0};
	ShapeType type;

	Parser_GetString(type_str, json, "type");

	type = ShapeType_GetTypeFromString(type_str);

	if(type == SHAPE_NONE) {
		return;
	}

	Shape *shape = Scene_AddShape(&data->scene, type);

	Parser_GetVec3(shape->k_diffuse, json, "diffuse");
	Parser_GetVec3(shape->k_specular, json, "specular");
	Parser_GetFloat(shape->shineness, json, "shineness");

	switch(type) {
		case SHAPE_SPHERE:
			Parser_GetVec3(shape->data._Sphere.center, json, "center");
			Parser_GetFloat(shape->data._Sphere.radius, json, "radius");
			break;

		case SHAPE_PLANE:
			Parser_GetVec3(shape->data._Plane.normal, json, "normal");
			Parser_GetFloat(shape->data._Plane.d, json, "dot");
			break;

		default:
			break;
	}
}

bool Parser_ParseFile(RenderData *data, const char *filename) {
	char *str;
	cJSON *json;

	str = Parser_GetFileStr(filename);

	if(str == NULL) return false;

	json = cJSON_Parse(str);

	if(json == NULL) {
		return false;
	}

	Parser_ParseExport(data, cJSON_GetObjectItemCaseSensitive(json, "export"));
	Parser_ParseCamera(data, cJSON_GetObjectItemCaseSensitive(json, "camera"));
	Parser_ParseLights(data, cJSON_GetObjectItemCaseSensitive(json, "lights"));
	Parser_ParseShapes(data, cJSON_GetObjectItemCaseSensitive(json, "shapes"));

	cJSON_Delete(json);

	return true;
}
