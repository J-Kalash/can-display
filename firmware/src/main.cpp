#include "stm32f4xx.h"

#include "font.h"
#include "lcd.h"
#include "can.h"
#include "tach_output.h"
#include "render.h"

uint32_t rpm = 9999;
uint32_t steer = 1234;
float wheelspeed[4];

void handle_focus_rpm(can_frame_t* frame)
{
	rpm = (256 * (frame->data8[4] & 0xf) + frame->data8[5]) * 2;
}
void handle_focus_steering(can_frame_t* frame)
{
	int tmp = (256 * (frame->data8[5] - 128)) + frame->data8[6];
	steer = tmp / 10 + 2000;
}

void handle_focus_wheel_speed(can_frame_t* frame)
{
	for(int i = 0; i < 4; i++)
	{
		uint8_t msb = frame->data8[2 * i] - 128;
		uint8_t lsb = frame->data8[2 * i + 1];

		wheelspeed[i] = (256 * msb + lsb) / 100.0f;
	}
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

void stringify(uint32_t n, char* str)
{
	str[0] = n / 1000 + '0';
	str[1] = (n / 100) % 10 + '0';
	str[2] = (n / 10) % 10 + '0';
	str[3] = n % 10 + '0';
	str[4] = 0;
}

int main(void)
{
	SystemCoreClockUpdate();

	Fonts_Init();

	//Tach_Init(8);

	CAN_Init();

	CAN_RegisterReceptionHandler(0x090, &handle_focus_rpm);
	CAN_RegisterReceptionHandler(0x0b0, &handle_focus_steering);
	CAN_RegisterReceptionHandler(0x190, &handle_focus_wheel_speed);

	LCD_Init();

	DMA2DRender r(framebuffer, 96, 64);

	//int n = 0;
	while(1)
	{
		uint32_t localRpm = rpm;
		uint32_t localSteer = steer;

		char strRpm[5];
		char strSteer[5];

		stringify(localRpm, strRpm);
		stringify(localSteer, strSteer);

		r.FillRect(0, 0, 96, 64, localRpm > 5800 ? 0xF800 : 0x0000);

		r.DrawString(0, 0, 0, strRpm, 0x000090ff);
		r.DrawString(0, 30, 0, strSteer, 0x000090ff);

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
