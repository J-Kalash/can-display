#include "stm32f0xx.h"

#include "can.h"
#include "tach_output.h"

uint16_t rpm;
int16_t clt, iat, ecu_temp;
float map, tps, afr, battery;
float timing, fuel_pw;

void handle_data1(can_frame_t* frame)
{
	rpm = frame->data16[0];
	map = (frame->data16[2]) / 10.0f;
	tps = ((short)frame->data16[3]) / 10.0f;

	Tach_SetOutput(rpm);
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

int main(void)
{
	Tach_Init(8);
	CAN_Init();

	CAN_RegisterReceptionHandler(0x500, &handle_data1);
	CAN_RegisterReceptionHandler(0x501, &handle_data2);
	CAN_RegisterReceptionHandler(0x502, &handle_data3);

	// Interrupts take it from here
	while(1);
}
