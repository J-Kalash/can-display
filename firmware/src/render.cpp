#include "render.h"

#include "stdint.h"

#include "stm32f4xx.h"

#include "font.h"

typedef struct character_s {
	uint8_t data[30];
} character_t;

void Render::DrawString(uint16_t x, uint16_t y, uint16_t kern, char* str, uint32_t color)
{
	while(*str != 0)
    {
    	font_character_t* c = &font_comicsans_30[*str - ' '];

    	int charHeightOffset = 0;
    	int charWidth = c->width;
    	int charHeight = c->height;

        this->CopyRectangleAndBlendBackground(c->pixels, x, y, charWidth, charHeight, color);

        x += charWidth + kern;
        str++;
    }
}

void DMA2DRender::CopyRectangleAndBlendBackground(const void* source, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color)
{
    this->ResetDMA2DAndMode(ModeMemToMemBlend);

    // Set the upper left corner of the source rectangle to copy
    DMA2D->FGMAR = (uintptr_t)source;
    // characters are in order of rows, so no row increment
    DMA2D->FGOR = 0;
    DMA2D->FGPFCCR = 0x9; // 0x9 = 0b1001 = A8 (alpha only)
    DMA2D->FGCOLR = color;

    // Now we set the background to the framebuffer, so that
    // it reads foreground, reads background (fb), blends the two,
    // then places the result in the framebuffer again.
    DMA2D->BGMAR = (uintptr_t)this->ComputeStartAddress(x, y);
    DMA2D->BGOR = this->width - width;  // Display width, not copy width
    // Do nothing fancy here, just set the color mode to the same as the
    // output color mode, since we're reading the framebuffer as the background
    DMA2D->BGPFCCR = 0x2;   // 2 = rgb565

    // Set PFC output color mode
    DMA2D->OPFCCR = 0x2;    // 2 = rgb565

    this->SetStartAddress(x, y);
    
    // At the end of the line we increment by one screen
    // width, minus fill region width
    DMA2D->OOR = this->width - width;

    // Set width/height of region to fill
    DMA2D->NLR = width << 16 | height;

    DMA2D->CR |= DMA2D_CR_START;

    // Wait for finish
    while(DMA2D->CR & DMA2D_CR_START) ;
}

DMA2DRender::DMA2DRender(uint16_t* framebuffer, uint16_t width, uint16_t height)
{
    this->framebuffer = framebuffer;
    this->width = width;
    this->height = height;

    // Turn it on
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA2DEN;

    // 16bpp, rgb565
    DMA2D->OPFCCR = 0x2;
}

void Render::SetPixel(uint16_t x, uint16_t y, uint16_t color)
{
    uint16_t* px = (uint16_t*)ComputeStartAddress(x, y);

    *px = color;
}

void DMA2DRender::FillRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color)
{
	this->ResetDMA2DAndMode(ModeRegToMem);

    // Set fill color
    DMA2D->OCOLR = color;
    
    // At the end of the line we increment by one screen
    // width, minus fill region width
    DMA2D->OOR = this->width - width;

    // Set width/height of region to fill
    DMA2D->NLR = width << 16 | height;
    
    this->SetStartAddress(x, y);
    DMA2D->CR |= DMA2D_CR_START;

    // Wait for finish
    while(DMA2D->CR & DMA2D_CR_START) ;
}

void DMA2DRender::ResetDMA2DAndMode(uint8_t mode)
{
    // Set the mode, and abort current operation
    DMA2D->CR = (mode & 0x3) << 16 | DMA2D_CR_ABORT;
}

void* Render::ComputeStartAddress(uint16_t x, uint16_t y)
{
    uint32_t px_offset = x + y * this->width;
    uintptr_t byte_offset = px_offset * sizeof(uint16_t);
    return this->framebuffer + byte_offset;
}

void DMA2DRender::SetStartAddress(uint16_t x, uint16_t y)
{
    // Set the start destination address to the top-left corner
    DMA2D->OMAR = (uintptr_t)(ComputeStartAddress(x, y));
}
