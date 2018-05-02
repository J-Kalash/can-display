
#ifndef CAN_H_
#define CAN_H_

#include "stdint.h"

#define CAN_FIFO_LENGTH 16
#define CAN_RECEPTION_HANDLER_COUNT 16

typedef struct can_frame_s {
	uint16_t id;
	uint16_t res0;
	//uint8_t notSent;

	union
	{
		uint8_t data8[8];
		uint16_t data16[4];
		uint32_t data32[2];
		float dataf[2];
	};
} can_frame_t;

typedef void (*CanFrameGenerator_t)(can_frame_t*);
typedef void (*CanFrameReceptionHandler_t)(can_frame_t*);

void CAN_Init();
int CAN_RegisterReceptionHandler(uint16_t id, CanFrameReceptionHandler_t handler);

void CAN_send(can_frame_t* f);

#endif
