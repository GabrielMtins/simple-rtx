#ifndef PARSER_H
#define PARSER_H

#include <cJSON.h>
#include <stdbool.h>

#include "Render.h"

bool Parser_ParseFile(RenderData *data, const char *filename);

#endif
