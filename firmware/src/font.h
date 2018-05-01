#ifndef _FONT_H_
#define _FONT_H_

#include "stdint.h"

typedef struct font_character_s {
	uint8_t width, height;
	const uint8_t* pixels;
} font_character_t;

extern font_character_t font_calibri_30[];
extern font_character_t font_comicsans_30[];
extern font_character_t font_cs_30[];


void Fonts_Init();

#endif
