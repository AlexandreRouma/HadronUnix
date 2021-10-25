#ifndef BOOTOPTS_H
#define BOOTOPTS_H

struct bootopts {
	char *kernel;
	char *initrd;
	char *cmd;
};
typedef struct bootopts bootopts_t;

void bootopts_fill(bootopts_t *bootopts, char *src);

#endif