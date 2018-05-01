#include "stm32f4xx.h"

#include "lcd.h"
#include "can.h"
#include "tach_output.h"
#include "render.h"

uint16_t rpm;
int16_t clt, iat, ecu_temp;
float map, tps, afr, battery;
float timing, fuel_pw;

void handle_data1(can_frame_t* frame)
{
	rpm = frame->data16[0];
	map = (frame->data16[2]) / 10.0f;
	tps = ((short)frame->data16[3]) / 10.0f;

	//Tach_SetOutput(rpm);
}

void handle_data2(can_frame_t* frame)
{
	clt = frame->data8[0] - 40;
	iat = frame->data8[1] - 40;

	afr = frame->data8[5] / 10.0f;
	battery = frame->data8[6] / 10.0f;
	ecu_temp = frame->data8[6] - 40;
}

void handle_data3(can_frame_t* frame)
{
	timing = frame->data16[0] / 100.0f;
	fuel_pw = frame->data16[1] / 1000.0f;
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

	//return c;
}

int main(void)
{
	SystemCoreClockUpdate();

	//Tach_Init(8);

	/*CAN_Init();

	CAN_RegisterReceptionHandler(0x500, &handle_data1);
	CAN_RegisterReceptionHandler(0x501, &handle_data2);
	CAN_RegisterReceptionHandler(0x502, &handle_data3);*/

	LCD_Init();

	DMA2DRender r(framebuffer, 96, 64);

	int n = 0;

	while(1)
	{
		char str[5];
		str[0] = n / 1000 + '0';
		str[1] = (n / 100) % 10 + '0';
		str[2] = (n / 10) % 10 + '0';
		str[3] = n % 10 + '0';
		str[4] = 0;

		r.FillRect(0, 0, 96, 64, 0x0000);

		r.DrawString(0, 0, 0, str, 0x00ffff00);

		r.DrawString(0, 30, 0, str, 0x0000ffff);

		LCD_WriteFramebuffer(framebuffer);


		n++;
		/*for(int i = 0; i < 1000000; i++)
		{
			__asm__("nop");
		}*/
	}


	// Interrupts take it from here
	while (1)
		;
}
