#ifndef SPI_H_
#define SPI_H_

#include "stdint.h"

void SPI_Init();

void SPI_TransactionBegin();
void SPI_TransactionEnd();

void SPI_Send(uint8_t tx);
uint8_t SPI_SendReceive(uint8_t tx);
void SPI_Send_DMA(uint8_t* ptr, uint16_t count);
#endif
