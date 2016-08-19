/*
 * main.h
 *
 *  Created on: Aug 10, 2016
 *      Author: Owner
 */

#ifndef MAIN_H_
#define MAIN_H_
#include "global.h"
#include "serial.h"
#include "machine.h"
#include "login.h"
void resetBuffer(buffer *buf);
void displayInfo(void);

void initGpio(void);
int encryptPointer;
void getFocus(buffer *buf);

void lpmTimer(void);
void initTimer(void);
void idle(void);
void noapp(int argc,char *argv[]);
void returntoCommand(void);
void toggleBuffer(void);
#endif /* MAIN_H_ */
