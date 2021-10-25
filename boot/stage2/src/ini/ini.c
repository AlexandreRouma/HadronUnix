#include "ini.h"

int eat_ws(ini_t *ini) {
	while (*ini->src != '\0' && (*ini->src == ' ' || *ini->src == '\n' || *ini->src == '\t'))
		ini->src++;
	return *ini->src == '\0';
}

int eat_until(ini_t *ini, char c) {
	while (*ini->src != '\0' && *ini->src != c)
		ini->src++;

	return *ini->src == '\0';
}

void ini_init(ini_t *ini, char *src) {
	*ini = (ini_t) {
		.src = src,
	};
}

int ini_iter(ini_t *ini, char **a, char **b, char **s) {
	if (*ini->src == '\0')
		return 0;

	*a = NULL;
	*b = NULL;
	*s = NULL;
	eat_ws(ini);

	switch (*ini->src) {
	case '[':
		ini->src++;
		*s = ini->src;
		if (!eat_until(ini, ']')) {
			*ini->src = '\0';
			ini->src++;
			eat_until(ini, '\n');
		} else {
			return 0;
		}
		break;
	case ';':
		if (!eat_until(ini, '\n'))
			ini->src++;
		break;
	default:
		*a = ini->src;
		if (eat_until(ini, '='))
			return 0;
		*ini->src = '\0';
		ini->src++;
		*b = ini->src;
		if (!eat_until(ini, '\n')) {
			*ini->src = '\0';
			ini->src++;
		}
	}
	return 1;
}
