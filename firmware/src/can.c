#include "can.h"

#include "stm32f0xx.h"

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
	RCC->APB1ENR |= RCC_APB1ENR_CANEN;

	// Enter init mode
	CAN->MCR |= CAN_MCR_INRQ;
	// Wait for init mode
	while ((CAN->MSR & CAN_MSR_INAK) == 0);

	CAN->BTR = (1 << 24) |	// SJW = 1
				(1 << 20) | // TS2 = 2
				(4 << 16) |	// TS1 = 5
				(3 << 0);   // BRP = 1/4

	CAN->MCR |= CAN_MCR_NART;

	// Leave init mode
	CAN->MCR &= ~CAN_MCR_INRQ;
	// Wait for end of init mode
	while (CAN->MSR & CAN_MSR_INAK);

	// Leave sleep
	CAN->MCR &= ~CAN_MCR_SLEEP;
	// Wait for the awakening
	while (CAN->MSR & CAN_MSR_SLAK);


	CAN->FMR |= CAN_FMR_FINIT;
	CAN->sFilterRegister[0].FR1 = 0;
	CAN->sFilterRegister[0].FR2 = 0;
	CAN->FA1R = 1;
	CAN->FMR &= ~CAN_FMR_FINIT;

	CAN->IER |= CAN_IER_FMPIE0;

	// Turn on GPIOs
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	GPIOA->AFR[1] |= 0x99 << 12;
	GPIOA->MODER |= GPIO_MODER_MODER11_1 | GPIO_MODER_MODER12_1;

	NVIC_EnableIRQ(CEC_CAN_IRQn);
}

void CEC_CAN_IRQHandler()
{
	// check that we rx'd something
	if(CAN->RF0R & CAN_RF0R_FMP0)
	{
		can_frame_t f;

		f.data32[1] = CAN->sFIFOMailBox[0].RDHR;
		f.data32[0] = CAN->sFIFOMailBox[0].RDLR;
		f.id = CAN->sFIFOMailBox[0].RIR >> 21;

		CAN_RecieveFrame(&f);

		// Release message
		CAN->RF0R |= CAN_RF0R_RFOM0;
	}
}
