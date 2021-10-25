#ifndef INI_H
#define INI_H

#include <stdint.h>
#include <stddef.h>

typedef struct ini {
	char *src;
} ini_t;

void ini_init(ini_t *ini, char *src);
int ini_iter(ini_t *ini, char **a, char **b, char **s);
void ini_end(ini_t *ini);

#endif
