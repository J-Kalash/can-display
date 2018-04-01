#ifndef TACH_OUTPUT_H_
#define TACH_OUTPUT_H_

#include "stdint.h"

/**
 * The cylinders parameter here doesn't care about
 * how many your engine actually has, but what the
 * tachometer is set to think you have.  If you have
 * a 4 cylinder, set the tach to 6, and call
 * Tach_Init(6), the tach will read correctly.
 * We recieve the true RPM over CAN, so we really
 * don't care (or know) how many cylinders the engine
 * really has.
 */
void Tach_Init(uint8_t cylinders);
void Tach_SetOutput(uint16_t rpm);

#endif
