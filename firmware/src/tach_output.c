#include "tach_output.h"

#include "stm32f0xx.h"

static uint8_t cyl = 4;

void Tach_Init(uint8_t cylinders)
{
	cyl = cylinders;

	// Turn power on
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
	// Turn on GPIO A for output channel
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;

	// Set up timer for tach output
	// No division, upcounter, not running
	TIM2->CR1 = 0;
	TIM2->CR2 = 0;

	TIM2->CNT = 0;
	// Divide F_CPU by 48 (47 + 1)
	TIM2->PSC = 47;

	// Set up PMW mode on TIM2 CH1 (PA0)
	TIM2->CCMR1 = TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1; // pwm mode 1

	// Set PA0 to AF1
	GPIOA->AFR[0] |= 0x1;
	// Set PA0 to alt func
	GPIOA->MODER |= GPIO_MODER_MODER0_1;
}

void Tach_SetOutput(uint16_t rpm)
{
	// Cancel output when turning slowly to avoid glitches
	/*if(rpm < 100)
	{
		// Stop counting
		TIM2->CR1 &= ~TIM_CR1_CEN;
		TIM2->CNT = 0;
		TIM2->

		return;
	}*/

	// We fake an RPM signal to have one rising edge
	// per cylinder firing event at 50% duty cycle
	uint32_t cylinders_per_minute = rpm * cyl / 2;

	// The timer runs at 1mhz, which is 60 million/min
	float period_cycles = 60000000.0f / cylinders_per_minute;
	uint32_t cycles = (uint32_t)period_cycles;

	// We set the auto reload register to full period
	TIM2->ARR = cycles;
	// And the compare register to period/2 (50% duty)
	TIM2->CCR1 = cycles >> 1;

	// start counter
	TIM2->CR1 |= TIM_CR1_CEN;
}
