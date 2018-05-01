#ifndef _RENDER_H_
#define _RENDER_H_

#include "stdint.h"

class Render
{
public:
    virtual void FillRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color) = 0;
    void SetPixel(uint16_t x, uint16_t y, uint16_t color);
    void* ComputeStartAddress(uint16_t x, uint16_t y);
    virtual void CopyRectangleAndBlendBackground(void* source, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color) = 0;
    void DrawString(uint16_t x, uint16_t y, uint16_t kern, char* str, uint32_t color);
protected:
    uint16_t width, height;
    void* framebuffer;
};


/*class SoftwareRender : public Render
{
public:
    void FillRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color);
};*/

typedef enum DMA2DMode_e {
    ModeMemToMem = 0x0,
    ModeMemToMemPFC = 0x1,
    ModeMemToMemBlend = 0x2,
    ModeRegToMem = 0x3,
} DMA2DMode;


class DMA2DRender : public Render
{
public:
    DMA2DRender(uint16_t* framebuffer, uint16_t width, uint16_t height);

    void FillRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color);
    void CopyRectangleAndBlendBackground(void* source, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color);
private:
    void ResetDMA2DAndMode(uint8_t mode);
    void SetStartAddress(uint16_t x, uint16_t y);
};

#endif
