#pragma once
#include <stdint.h>
#include <stddef.h>

struct ini {
	char *src;
};
typedef struct ini ini_t;

void ini_init(ini_t *ini, char *src);
int ini_iter(ini_t *ini, char **a, char **b, char **s);
void ini_end(ini_t *ini);