/* 
 * 240p Test Suite for Nintendo 64
 * Copyright (C)2024 Artemio Urbina
 *
 * This file is part of the 240p Test Suite
 *
 * The 240p Test Suite is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * The 240p Test Suite is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with 240p Test Suite; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA	02111-1307	USA
 */
 
 #include "image.h"
 #include "video.h"

#define PAL4_SIZE	16
#define PAL8_SIZE	256

bool clearScreen = false;
unsigned int currFB = 0;

#ifndef USE_PRESCALE
// Upscale frame buffer
surface_t *__real_disp = NULL;
surface_t *__upscale_fb = NULL;
bool upscaleFrame = false;
bool menuIgnoreUpscale = false;
#endif

void rdpqSetDrawMode() {
	if(vMode == SUITE_640x480) {
		rdpq_set_mode_standard();
		rdpq_mode_alphacompare(1);
		rdpq_mode_filter(FILTER_POINT);
	}
	else
		rdpq_set_mode_copy(true);
}
 
void rdpqStart() {
	if(clearScreen) {
		rdpq_attach_clear(__disp, NULL);
#ifndef USE_PRESCALE
		if(upscaleFrame) {
			rdpq_attach_clear(__upscale_fb, NULL);
			rdpq_detach_wait();
		}
#endif
		if(currFB == current_buffers) {
			clearScreen = false;
			currFB = 0;
		}
		else
			currFB ++;
	}
	else
		rdpq_attach(__disp, NULL);
	
	rdpqSetDrawMode();
}
 
void rdpqEnd() {
#ifndef USE_PRESCALE
	if(upscaleFrame) {
		__real_disp = __disp;
		__disp = __upscale_fb;
	}
#endif

	rdpq_detach_wait();
}
 
void rdpqDrawImage(image* data) {
#ifdef DEBUG_BENCHMARK
	assertf(data, "rdpqDrawImage() received NULL image");
	assertf(data->tiles, "rdpqDrawImage() received NULL tiles");
#else
	if(!data) 				return;
	if(!data->tiles)		return;
#endif

#ifdef USE_PRESCALE
	if(vMode == SUITE_640x480 && data->scale) {
		if(data->center) {
			data->x = (dW/2 - data->tiles->width)/2;
			data->y = (dH/2 - data->tiles->height)/2;
		}
		rdpq_sprite_blit(data->tiles, 2*data->x, 2*data->y, &(rdpq_blitparms_t) {
			.scale_x = 2.0f, .scale_y = 2.0f
			});
	}
	else {
		if(data->center) {
			data->x = (dW - data->tiles->width)/2;
			data->y = (dH - data->tiles->height)/2;
		}
		rdpq_sprite_blit(data->tiles, data->x, data->y, NULL);
	}
#else
	if(vMode == SUITE_640x480 && !data->scale)
		upscaleFrame = false;
	
	if(upscaleFrame)
		rdpq_attach(__upscale_fb, NULL);
	
	if(data->center) {
		const surface_t *output = rdpq_get_attached();
		
		if(output) {
			data->x = (output->width - data->tiles->width)/2;
			data->y = (output->height - data->tiles->height)/2;
		}
	}
	rdpq_sprite_blit(data->tiles, data->x, data->y, NULL);
	
	if(upscaleFrame)
		rdpq_detach();
#endif
}

void rdpqDrawImageXY(image* data, int x, int y) {
#ifdef DEBUG_BENCHMARK
	assertf(data, "rdpqDrawImage() received NULL image");
	assertf(data->tiles, "rdpqDrawImage() received NULL tiles");
#else
	if(!data) 				return;
	if(!data->tiles)		return;
#endif
	
#ifdef USE_PRESCALE
	if(vMode == SUITE_640x480 && data->scale) {
		rdpq_sprite_blit(data->tiles, 2*x, 2*y, &(rdpq_blitparms_t) {
			.scale_x = 2.0f, .scale_y = 2.0f
			});
	}
	else
		rdpq_sprite_blit(data->tiles, x, y, NULL);
#else
	if(vMode == SUITE_640x480 && !data->scale)
		upscaleFrame = false;
		
	if(upscaleFrame)
		rdpq_attach(__upscale_fb, NULL);
	
	rdpq_sprite_blit(data->tiles, x, y, NULL);
	
	if(upscaleFrame)
		rdpq_detach();
#endif
}
 
void rdpqClearScreen() {
	rdpq_set_mode_fill(RGBA32(0, 0, 0, 0xff));
	rdpq_fill_rectangle(0, 0, dW, dH);
	
	rdpqSetDrawMode();
}

/* Image stuct */

image *loadImage(char *name) {
	image *data = NULL;
	
	data = (image*)malloc(sizeof(image));
	if(!data)
		return NULL;
	data->tiles = sprite_load(name);
	if(!data->tiles) {
		free(data);
		return(NULL);
	}
	data->x = 0;
	data->y = 0;
	data->center = false;
	data->scale = true;
	
	data->palette = NULL;
	data->origPalette = NULL;
	data->palSize = 0;
	data->fadeSteps = 0;
	
	/* prepare palette if it exists */
	tex_format_t texFormat = sprite_get_format(data->tiles);
	
	if(texFormat == FMT_CI4)
		data->palSize = PAL4_SIZE;
	if(texFormat == FMT_CI8)
		data->palSize = PAL8_SIZE;

	if(data->palSize) {
		data->palette = sprite_get_palette(data->tiles);
#ifdef DEBUG_BENCHMARK
		assertf(data->palette != NULL, 
			"loadImage() No palette in indexed image");
#endif
	}
	
	return(data);
}
 
void freeImage(image **data) {
	if(*data) {
		if((*data)->tiles) {
			sprite_free((*data)->tiles);
			(*data)->tiles = NULL;
		}
		if((*data)->origPalette) {
			free((*data)->origPalette);
			(*data)->origPalette = NULL;
		}
		free(*data);
		*data = NULL;
	}
}

/* Frame Buffer for menu */

surface_t *__menu_fb = NULL;

bool copyMenuFB() {	
	if(!__disp)
		return false;
	
#ifdef DEBUG_BENCHMARK
	assertf(__menu_fb == NULL, "copyFrameBuffer(): __menu_fb was not NULL");
#else
	freeMenuFB();
#endif

	__menu_fb = (surface_t *)malloc(sizeof(surface_t));
	if(!__menu_fb)
		return false;
	//memset(__menu_fb, 0, sizeof(surface_t));
		
	*__menu_fb = surface_alloc(surface_get_format(__disp), __disp->width, __disp->height);
	if(!__menu_fb->buffer) {
		free(__menu_fb);
		__menu_fb = NULL;
		return false;
	}
	
	rdpq_attach(__menu_fb, NULL);
	rdpq_set_mode_copy(false);
	rdpq_tex_blit(__disp, 0, 0, NULL);
	rdpq_detach_wait();

#ifndef USE_PRESCALE
	if(!upscaleFrame)
		menuIgnoreUpscale = true;
#endif
	return true;
}

void freeMenuFB() {
	if(__menu_fb) {
		surface_free(__menu_fb);
		free(__menu_fb);
		__menu_fb = NULL;
	}
#ifndef USE_PRESCALE
	if(menuIgnoreUpscale)
		menuIgnoreUpscale = false;
#endif
}

void drawMenuFB() {
	if(!__menu_fb || !__disp)
		return;

#ifndef USE_PRESCALE
	rdpq_attach(upscaleFrame ? __upscale_fb : __disp, NULL);
#else
	rdpq_attach(__disp, NULL);
#endif
	rdpq_set_mode_copy(false);
	rdpq_tex_blit(__menu_fb, 0, 0, NULL);
	rdpq_detach_wait();
}

void darkenMenuFB(int amount) {
	if(!__menu_fb)
		return;

	if(surface_get_format(__menu_fb) != FMT_RGBA16)
		return;
	
	uint16_t *pixels = (uint16_t*)__menu_fb->buffer;
	unsigned int len = TEX_FORMAT_PIX2BYTES(surface_get_format(__menu_fb), __menu_fb->width * __menu_fb->height)/2;

	for(unsigned int i = 0; i < len; i++) {
		color_t color;
			
		color = color_from_packed16(pixels[i]);
		
		if(color.r > amount)
			color.r -= amount;
		else
			color.r = 0;
		if(color.g > amount)
			color.g -= amount;
		else
			color.g = 0;
		if(color.b > amount)
			color.b -= amount;
		else
			color.b = 0;
	
		pixels[i] = graphics_convert_color(color);
	}
}

#ifndef USE_PRESCALE
/* Frame Buffer for upscaling */

void rdpqUpscalePrepareFB() {
	if(!menuIgnoreUpscale && vMode == SUITE_640x480) {
		createUpscaleFB();
		upscaleFrame = true;
	}
}

bool createUpscaleFB() {
	if(__upscale_fb || !__disp)
		return false;
	
	__upscale_fb = (surface_t *)malloc(sizeof(surface_t));
	if(!__upscale_fb)
		return false;
	memset(__upscale_fb, 0, sizeof(surface_t));

	*__upscale_fb = surface_alloc(surface_get_format(__disp), __disp->width/2, __disp->height/2);
	if(!__upscale_fb->buffer) {
		free(__upscale_fb);
		__upscale_fb = NULL;
		return false;
	}
	
	return true;
}

void freeUpscaleFB() {
	if(__upscale_fb) {
		surface_free(__upscale_fb);
		free(__upscale_fb);
		__upscale_fb = NULL;
	}
	
	if(__real_disp) {
		__disp = __real_disp;
		__real_disp = NULL;
	}
}

void executeUpscaleFB() {
	if(!upscaleFrame)
		return;

	if(!__upscale_fb || !__disp)
		return;
	
	if(__real_disp) {
		__disp = __real_disp;
		__real_disp = NULL;
	}
	
	rdpq_attach(__disp, NULL);
	rdpq_set_mode_standard();
	rdpq_mode_alphacompare(0);
	rdpq_mode_filter(FILTER_POINT);
	rdpq_tex_blit(__upscale_fb, 0, 0, &(rdpq_blitparms_t) {
			.scale_x = 2.0f, .scale_y = 2.0f
			});
	rdpq_detach_wait();
	
	upscaleFrame = false;
}
#endif

/* Fade paletted images */

void fadePaletteStep(uint16_t *colorRaw, unsigned int fadeSteps) {
	color_t color;
			
	color = color_from_packed16(*colorRaw);
	color.r = (color.r > 0) ? (color.r - color.r/fadeSteps) : 0;
	color.g = (color.g > 0) ? (color.g - color.g/fadeSteps) : 0;
	color.b = (color.b > 0) ? (color.b - color.b/fadeSteps) : 0;
	*colorRaw = color_to_packed16(color);
}

void setPaletteFX(image *data) {
	if(!data->palette)
		return;
	
	if(data->origPalette) {
		free(data->origPalette);
		data->origPalette = NULL;
	}
	
	if(data->palette) {
		data->origPalette = (uint16_t *)malloc(sizeof(uint16_t)*data->palSize);
		if(data->origPalette)
			memcpy(data->origPalette, data->palette, sizeof(uint16_t)*data->palSize);
	}
}

void resetPalette(image *data) {
	if(data->palette && data->origPalette) {
		memcpy(data->palette, data->origPalette, sizeof(uint16_t)*data->palSize);
		data_cache_hit_writeback(data->palette, sizeof(uint16_t)*data->palSize);
	}
}

void fadeInit(image *data, unsigned int steps) {
#ifdef DEBUG_BENCHMARK
	assertf(data, "fadeInit() NULL pointer received");
	assertf(data->palette, "fadeInit() Image with no palette in fade");
	assertf(steps != 0, "fadeInit() steps is zero");
#else
	if(!data || !data->palette || steps == 0)
		return;
#endif

	resetPalette(data);
	data->fadeSteps = steps;
}

void fadeImageStep(image *data) {
#ifdef DEBUG_BENCHMARK
	assertf(data, "fadeImageStep() NULL pointer received");
	assertf(data->palette, "fadeImageStep() Image with no palette for dade");
	assertf(data->fadeSteps != 0, "fadeImageStep() fadeStep is zero");
#else
	if(!data || !data->palette || data->fadeSteps == 0)
		return;
#endif

	for(unsigned int c = 0; c < data->palSize; c++)
		fadePaletteStep(&data->palette[c], data->fadeSteps);
	
	// Invalidate the cache
	data_cache_hit_writeback_invalidate(data->palette, sizeof(uint16_t)*data->palSize);
}

#ifdef DEBUG_BENCHMARK
#include "font.h"

void printPalette(image *data, int x, int y) {
	for(unsigned int c = 0; c < data->palSize; c++) {
		char str[64];
		color_t color = color_from_packed16(data->palette[c]);
	
		sprintf(str, "%X %X %X", color.r, color.g, color.b);
		drawStringS(x, y, 0x00, 0xff, 0x00, str);
		y += fh;
	}
}
#endif