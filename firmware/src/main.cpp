#include "stm32f4xx.h"

#include "font.h"
#include "lcd.h"
#include "can.h"
#include "tach_output.h"
#include "render.h"

uint32_t rpm = 9999;

void handle_focus_rpm(can_frame_t* frame)
{
	rpm = (256 * (frame->data8[4] - 96) + frame->data8[5]) * 2;
}

#include "stdint.h"

uint16_t framebuffer[96 * 64];

uint16_t rgb(uint8_t r, uint8_t g, uint8_t b) {
	uint16_t c;
	  c = r >> 3;
	  c <<= 6;
	  c |= g >> 2;
	  c <<= 5;
	  c |= b >> 3;
	return c;
}

int main(void)
{
	SystemCoreClockUpdate();

	Fonts_Init();

	//Tach_Init(8);

	CAN_Init();

	CAN_RegisterReceptionHandler(0x090, &handle_focus_rpm);

	LCD_Init();

	DMA2DRender r(framebuffer, 96, 64);

	//int n = 0;

	while(1)
	{
		int n = rpm;

		char str[5];
		str[0] = n / 1000 + '0';
		str[1] = (n / 100) % 10 + '0';
		str[2] = (n / 10) % 10 + '0';
		str[3] = n % 10 + '0';
		str[4] = 0;

		r.FillRect(0, 0, 96, 64, 0x0000);

		r.DrawString(0, 0, 0, str, 0x00ff8000);
		r.DrawString(0, 30, 0, str, 0x000080ff);

		LCD_WriteFramebuffer(framebuffer);

		/*can_frame_t f;
		f.data32[0] = 0;
		f.data32[1] = 0;

		f.data8[5] = 0xE2;
		f.data8[4] = 96 + 4;
		f.id = 0x090;

		CAN_send(&f);*/

	}


	// Interrupts take it from here
	while (1)
		;
}
