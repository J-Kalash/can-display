#ifndef _FONT_H_
#define _FONT_H_


typedef struct font_character_s {
	uint8_t width, height;
	uint8_t* pixels;
} font_character_t;


#endif
