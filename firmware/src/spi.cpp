
#include "stm32f4xx.h"

#include "spi.h"


void SPI_Init()
{
	// Setup SPI
	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

	SPI1->CR1 =	SPI_CR1_CPHA |
				SPI_CR1_CPOL |
				SPI_CR1_SSM |	// we manage CS pin ourselves
				SPI_CR1_SSI |
				2 << 3 |		// fpclk / 8 = ~10.5mbit/s
				SPI_CR1_MSTR;	// SPI master
	SPI1->CR2 = SPI_CR2_TXDMAEN;

	SPI1->CR1 |= SPI_CR1_SPE;	// Enable SPI

	// Setup DMA
	RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;


	// Plumb GPIOs
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIODEN;
	GPIOA->AFR[0] |= 0x50500000;	// AF5 for PA5/7
	GPIOA->OSPEEDR |= 0xCC00;	// High speed PA5/7
	GPIOA->MODER |= 0x8800;		// PA5/7 = alt func

	GPIOD->MODER |= 0x10000000;   // PD14 = gen. output (chip select)

	// Set CS to idle
	SPI_TransactionEnd();
}

void SPI_TransactionBegin()
{
	GPIOD->BSRRH = 1 << 14;
}

void SPI_TransactionEnd()
{
	// wait for not busy
	while(READ_BIT(SPI1->SR, SPI_SR_BSY));

	GPIOD->BSRRL = 1 << 14;
}

inline void SPI_Send(uint8_t tx)
{
	// wait for not busy
	while(READ_BIT(SPI1->SR, SPI_SR_BSY));

	*(uint8_t*)(&SPI1->DR) = tx;

	// wait for not busy
	while(READ_BIT(SPI1->SR, SPI_SR_BSY));
}

uint8_t SPI_SendReceive(uint8_t tx)
{
	SPI_Send(tx);

	return *(uint8_t*)(&SPI1->DR);
}

void SPI_Send_DMA(uint8_t* ptr, uint16_t count)
{
	// put SPI in 16 bit mode
	SPI1->CR1 &= ~SPI_CR1_SPE;	// disable
	SPI1->CR1 |= SPI_CR1_DFF;	// set to 16-bit
	SPI1->CR1 |= SPI_CR1_SPE;	// enable

	// reset
	DMA2->LIFCR |= DMA_LIFCR_CTCIF3 | DMA_LIFCR_CHTIF3;
	DMA2_Stream3->CR = 0;

	DMA2_Stream3->CR = 3 << 25 |	// channel 3
						   DMA_SxCR_MSIZE_0 | // 16 bit transfers (2 bytes) for memory
						   DMA_SxCR_PSIZE_0 | // 16 bit transfers (2 bytes) for peripheral
						   DMA_SxCR_PL_1 |	// high priority
						   DMA_SxCR_MINC |	// increment memory addr
						   DMA_SxCR_DIR_0; // mem -> peripheral

	DMA2_Stream3->M0AR = (uintptr_t)ptr;
	DMA2_Stream3->PAR = (uintptr_t)&SPI1->DR;
	DMA2_Stream3->NDTR = count / 2;

	// hit it
	DMA2_Stream3->CR |= DMA_SxCR_EN;

	// wait for complete
	while(!READ_BIT(DMA2->LISR, DMA_LISR_TCIF3));

	// put SPI back in 8 bit mode
	SPI1->CR1 &= ~SPI_CR1_SPE;	// disable
	SPI1->CR1 &= ~SPI_CR1_DFF;	// set to 16-bit
	SPI1->CR1 |= SPI_CR1_SPE;	// enable
}
