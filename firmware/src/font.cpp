#include "font.h"
#include <string.h>


uint8_t* ptrRam3 = (uint8_t*)0x20020000;

static void InitFont(font_character_t* font)
{
	for(char c = ' '; c <= 'z'; c++)
	{
		// How large is this character?
		uint16_t pxCount = font->height * font->width;

		// Check that we won't overrun the end of the RAM region
		if((uintptr_t)ptrRam3 + pxCount > 0x2002FFFF)
		{
			return;
		}

		// Copy the character from flash to ram3
		memcpy(ptrRam3, font->pixels, pxCount);

		// Update pointer to pixel data in the font table
		font->pixels = ptrRam3;

		// Increment allocation pointer
		ptrRam3 += pxCount;
	}
}

void Fonts_Init()
{
	InitFont(font_calibri_30);
	InitFont(font_cs_30);
	InitFont(font_comicsans_30);
}
