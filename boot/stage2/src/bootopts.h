#ifndef BOOTOPTS_H
#define BOOTOPTS_H

typedef struct bootopts {
	char *kernel;
	char *initrd;
	char *cmd;
} bootopts_t;

void bootopts_fill(bootopts_t *bootopts, char *src);

#endif