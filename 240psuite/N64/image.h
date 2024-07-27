#ifndef IMAGE_H
#define IMAGE_H

#include "system.h"

typedef struct image_st {
	sprite_t *tiles;
	int	x;
	int y;
	bool center;
	bool scale;
	
	uint16_t *palette;
	uint16_t *origPalette;
	unsigned int palSize;
	unsigned int fadeSteps;
} image;

extern bool clearScreen;

void rdpqStart();
void rdpqEnd();
void rdpqDrawImage(image* data);
void rdpqDrawImageXY(image* data, int x, int y);
void rdpqClearScreen();

image *loadImage(char *name);
void freeImage(image **data);

bool copyMenuFB();
void freeMenuFB();
void drawMenuFB();
void darkenMenuFB(int amount);

#ifndef USE_PRESCALE
void rdpqUpscalePrepareFB();
bool createUpscaleFB();
void freeUpscaleFB();
void executeUpscaleFB();
#endif

void setPaletteFX(image *data);
void fadeInit(image *data, unsigned int steps);
void fadeImageStep(image *data);

#ifdef DEBUG_BENCHMARK
void printPalette(image *data, int x, int y);
#endif

#endif