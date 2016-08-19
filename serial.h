/*
 * serial.h
 *
 *  Created on: Aug 16, 2016
 *      Author: Owner
 */

#ifndef SERIAL_H_
#define SERIAL_H_
#include "global.h"
void writeDirect(uint8_t *data,int length);
uint8_t readByteDirect(void);
void initSerial(void);
#endif /* SERIAL_H_ */
