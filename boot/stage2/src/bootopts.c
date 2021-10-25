#include "bootopts.h"
#include <ini/ini.h>

// TODO: Switch with proper string.h
int streq(char *a, char *b) {
	int i;
	for (i = 0; a[i] && a[i] && a[i] == b[i]; i++);
	return a[i] == b[i];
}

void bootopts_fill(bootopts_t *bootopts, char *src) {
	// Set default options
	bootopts->kernel = "KERNEL";
	bootopts->initrd = "INITRD";
	bootopts->cmdline = "";

	// Initialize INI parser
	ini_t ini;
	ini_init(&ini, src);

	// Parse the INI and fill in the boot options
	char* section;
	char* key;
	char* value;
	while (ini_iter(&ini, &key, &value, &section)) {
		// Skip invalid entries
		if (!key || !value) { continue; }
		
		// Set option according to the key
		if (streq(key, "kernel")) {
			bootopts->kernel = value;
		}
		else if (streq(key, "initrd")) {
			bootopts->initrd = value;
		}
		else if (streq(key, "cmd")) {
			bootopts->cmdline = value;
		}
	}
}