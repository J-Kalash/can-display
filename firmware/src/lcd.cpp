#include "stm32f4xx.h"

#include "lcd.h"

#include "spi.h"

static inline void cmd() {
	GPIOA->BSRRH = 1 << 6;
}

static inline void data() {
	GPIOA->BSRRL = 1 << 6;
}

static void writeCommand(uint8_t c) {
	cmd();

	SPI_TransactionBegin();
	SPI_Send(c);
	SPI_TransactionEnd();
}

static void writeData(uint8_t c) {
	data();

	SPI_TransactionBegin();
	SPI_Send(c);
	SPI_TransactionEnd();
}

void wait()
{
	for(int i = 0; i < 1000000; i++)
	{
		__asm__("nop");
	}
}

void LCD_Init() {
	SPI_Init();

	// PA6 = gpio output
	GPIOA->MODER |= 1 << 12;
	GPIOA->MODER |= 1 << 6; // pa3 = reset

	// Toggle RST low to reset; CS low so it'll listen to us
	SPI_TransactionBegin();

	// Reset OLED
	{
		GPIOA->BSRRL = 1 << 3;	// high
		wait();
		GPIOA->BSRRH = 1 << 3;	// low
		wait();
		GPIOA->BSRRL = 1 << 3;	// high
		wait();
	}




	// Initialization Sequence
	writeCommand(SSD1331_CMD_DISPLAYOFF);  	// 0xAE
	writeCommand(SSD1331_CMD_SETREMAP); 	// 0xA0
	//writeCommand(0x32);
	writeCommand(0x72);
	/*#if defined SSD1331_COLORORDER_RGB
	writeCommand(0x72);				// RGB Color
#else
	writeCommand(0x76);				// BGR Color
#endif*/
	writeCommand(SSD1331_CMD_STARTLINE); 	// 0xA1
	writeCommand(0x0);
	writeCommand(SSD1331_CMD_DISPLAYOFFSET); 	// 0xA2
	writeCommand(0x0);
	writeCommand(SSD1331_CMD_NORMALDISPLAY);  	// 0xA4
	writeCommand(SSD1331_CMD_SETMULTIPLEX); 	// 0xA8
	writeCommand(0x3F);  			// 0x3F 1/64 duty
	writeCommand(SSD1331_CMD_SETMASTER);  	// 0xAD
	writeCommand(0x8E);
	writeCommand(SSD1331_CMD_POWERMODE);  	// 0xB0
	writeCommand(0x0B);
	writeCommand(SSD1331_CMD_PRECHARGE);  	// 0xB1
	writeCommand(0x31);
	writeCommand(SSD1331_CMD_CLOCKDIV);  	// 0xB3
	writeCommand(0xF0); // 7:4 = Oscillator Frequency, 3:0 = CLK Div Ratio (A[3:0]+1 = 1..16)
	writeCommand(SSD1331_CMD_PRECHARGEA);  	// 0x8A
	writeCommand(0x64);
	writeCommand(SSD1331_CMD_PRECHARGEB);  	// 0x8B
	writeCommand(0x78);
	writeCommand(SSD1331_CMD_PRECHARGEA);  	// 0x8C
	writeCommand(0x64);
	writeCommand(SSD1331_CMD_PRECHARGELEVEL);  	// 0xBB
	writeCommand(0x3A);
	writeCommand(SSD1331_CMD_VCOMH);  		// 0xBE
	writeCommand(0x3E);
	writeCommand(SSD1331_CMD_MASTERCURRENT);  	// 0x87
	writeCommand(0x06);
	writeCommand(SSD1331_CMD_CONTRASTA);  	// 0x81
	writeCommand(0x91);
	writeCommand(SSD1331_CMD_CONTRASTB);  	// 0x82
	writeCommand(0x50);
	writeCommand(SSD1331_CMD_CONTRASTC);  	// 0x83
	writeCommand(0x7D);
	writeCommand(SSD1331_CMD_DISPLAYON); //--turn on oled panel
}

static void goTo(int x, int y) {
  if ((x >= WIDTH) || (y >= HEIGHT)) return;

  // set x and y coordinate
  writeCommand(SSD1331_CMD_SETCOLUMN);
  writeCommand(x);
  writeCommand(WIDTH-1);

  writeCommand(SSD1331_CMD_SETROW);
  writeCommand(y);
  writeCommand(HEIGHT-1);
}

void LCD_drawPixel(uint16_t x, uint16_t y, uint16_t color) {
	goTo(x, y);

	data();

	SPI_TransactionBegin();

	SPI_Send(color >> 8);
	SPI_Send(color & 0xFF);

	SPI_TransactionEnd();
}

void LCD_WriteFramebuffer(uint16_t* fb) {
	goTo(0, 0);

	data();

	SPI_TransactionBegin();

	SPI_Send_DMA((uint8_t*)fb, sizeof(*fb) * WIDTH * HEIGHT);

	SPI_TransactionEnd();
}





