#include "bootopts.h"

#include <ini/ini.h>

int streq(char *a, char *b) {
	int i;
	for (i = 0; a[i] && a[i] && a[i] == b[i]; i++);
	return a[i] == b[i];
}

void bootopts_fill(bootopts_t *bootopts, char *src) {
	bootopts->kernel = "KERNEL";
	bootopts->initrd = "INITRD";
	bootopts->cmdline = "";

	ini_t ini;
	ini_init(&ini, src);
	char *a, *b, *s;
	while (ini_iter(&ini, &a, &b, &s)) {
		if (a && b) {
			if (streq(a, "kernel"))
				bootopts->kernel = b;
			else if (streq(a, "initrd"))
				bootopts->initrd = b;
			else if (streq(a, "cmd"))
				bootopts->cmd = b;
		}
	}
}