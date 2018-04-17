#include "can.h"

#include "stm32f4xx.h"

typedef struct can_handler_entry_s {
	uint16_t id;
	CanFrameReceptionHandler_t handler;
	//void* ref;
} can_handler_entry_t;

static can_handler_entry_t handlers[CAN_RECEPTION_HANDLER_COUNT];
static int lastHandlerIdx = 0;

int CAN_RegisterReceptionHandler(uint16_t id, CanFrameReceptionHandler_t handler)
{
	if(lastHandlerIdx == CAN_RECEPTION_HANDLER_COUNT - 1)
	{
		// We've filled the array already, do nothing
		return 1;
	}

	int idx = lastHandlerIdx++;

	handlers[idx].handler = handler;
	handlers[idx].id = id;

	return 0;
}

static void CAN_RecieveFrame(can_frame_t* frame)
{
	// Hunt for the right handler
	for(int i = 0; i <= lastHandlerIdx; i++)
	{
		// Check if this handler matches the one we're looking for
		if(handlers[i].id == frame->id)
		{
			CanFrameReceptionHandler_t h = handlers[i].handler;

			// Only call if not null
			if(h != 0)
			{
				h(frame);

				return;
			}
		}
	}
}



void CAN_Init()
{
	// Turn on CAN peripheral
	RCC->APB1ENR |= RCC_APB1ENR_CAN1EN;

	// Enter init mode
	CAN1->MCR |= CAN_MCR_INRQ;
	// Wait for init mode
	while ((CAN1->MSR & CAN_MSR_INAK) == 0);

	CAN1->BTR = (1 << 24) |		// SJW = 1
				(4 << 20) | 	// TS2 = 4
				(11 << 16) |	// TS1 = 11
				(5 << 0);   	// BRP = 1/4

	CAN1->MCR |= CAN_MCR_NART;

	// Leave init mode
	CAN1->MCR &= ~CAN_MCR_INRQ;
	// Wait for end of init mode
	while (CAN1->MSR & CAN_MSR_INAK);

	// Leave sleep
	CAN1->MCR &= ~CAN_MCR_SLEEP;
	// Wait for the awakening
	while (CAN1->MSR & CAN_MSR_SLAK);


	CAN1->FMR |= CAN_FMR_FINIT;
	CAN1->sFilterRegister[0].FR1 = 0;
	CAN1->sFilterRegister[0].FR2 = 0;
	CAN1->FA1R = 1;
	CAN1->FMR &= ~CAN_FMR_FINIT;

	CAN1->IER |= CAN_IER_FMPIE0;

	// Turn on GPIOs PB8/9 to AF4 (CAN)
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
	GPIOB->AFR[1] |= 0x44;
	GPIOB->MODER |= GPIO_MODER_MODER8_1 | GPIO_MODER_MODER9_1;

	NVIC_EnableIRQ(CAN1_RX0_IRQn);
}

void CEC_CAN_IRQHandler()
{
	// check that we rx'd something
	if(CAN1->RF0R & CAN_RF0R_FMP0)
	{
		can_frame_t f;

		f.data32[1] = CAN1->sFIFOMailBox[0].RDHR;
		f.data32[0] = CAN1->sFIFOMailBox[0].RDLR;
		f.id = CAN1->sFIFOMailBox[0].RIR >> 21;

		CAN_RecieveFrame(&f);

		// Release message
		CAN1->RF0R |= CAN_RF0R_RFOM0;
	}
}
