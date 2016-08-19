/*
 * -------------------------------------------
 *    MSP432 DriverLib - v3_10_00_09 
 * -------------------------------------------
 *
 * --COPYRIGHT--,BSD,BSD
 * Copyright (c) 2014, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * --/COPYRIGHT--*/
/******************************************************************************
 * MSP432 Command Line OS
 *
 * Description: POC of a terminal interface
 *
 *                MSP432P401
 *             ------------------
 *         /|\|                  |
 *          | |                  |
 *          --|RST               |
 *            |                  |
 *            |                  |
 *            |                  |
 *            |                  |
 *            |                  |
 * Author: 
*******************************************************************************/
/* DriverLib Includes */
#include "main.h"
/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#ifdef ewarm
#pragma data_alignment=1024
#else
#pragma DATA_ALIGN(controlTable, 1024)
#endif
const uint32_t DATA_RECEIVE=0xA5A2CAA5;
const uint32_t DATA_TRANSFER=0xB5B2CBB5;
statushandler machinehandler,loginhandler;
mainhandler processor;
application run;
uint8_t controlTable[1024];
buffer pausedqueue[2];
uint_fast8_t powerState;
int main(void)
{
    /* Stop Watchdog  */
    MAP_WDT_A_holdTimer();
    MAP_PCM_setCoreVoltageLevel(PCM_VCORE1);
    MAP_FlashCtl_setWaitState(FLASH_BANK0, 2);
    MAP_FlashCtl_setWaitState(FLASH_BANK1, 2);
    CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_48);
    if(!MAP_FlashCtl_unprotectSector(FLASH_MAIN_MEMORY_SPACE_BANK1,FLASH_SECTOR0|FLASH_SECTOR1))
    	while(1);
    uint32_t ptr;
    ptr = NAME_SET_FLAG_ADDRESS;
    if(!MAP_FlashCtl_eraseSector(ptr))
    	while(1);
    ptr = PASSWORD_SET_FLAG_ADDRESS;
    if(!MAP_FlashCtl_eraseSector(ptr))
    	while(1);
    processor=idle;
    run = noapp;
    ipc = malloc(sizeof(ipc));
	ipc->argv = malloc(3*sizeof(char*));
    active=&pausedqueue[0];
    if(!active)
    {

    }
    initSerial();
    initMachine();
    CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_12);
    CS_initClockSignal(CS_MCLK,CS_DCOCLK_SELECT,CS_CLOCK_DIVIDER_1);
    CS_initClockSignal(CS_SMCLK,CS_DCOCLK_SELECT,CS_CLOCK_DIVIDER_4);
    CS_initClockSignal(CS_ACLK,CS_REFOCLK_SELECT,CS_CLOCK_DIVIDER_1);
	MAP_PCM_setPowerState(PCM_AM_LDO_VCORE0);
    MAP_FlashCtl_setWaitState(FLASH_BANK0, 2);
    MAP_FlashCtl_setWaitState(FLASH_BANK1, 2);
	initTimer();
    initGpio();
	MAP_SysCtl_enableSRAMBankRetention(SYSCTL_SRAM_BANK1);
	MAP_Interrupt_enableMaster();
	MAP_PCM_gotoLPM0();
    while(1)
    {
    	processor();
    	run(ipc->argc,ipc->argv);
    	MAP_PCM_setPowerState(powerState);
    }
}
void idle(void){;}
void noapp(int argc,char *argv[]){;}
void echo(int argc, char *argv[]){
	bool exit=false;
	while(!exit){
		if(active->readHEAD!=active->receiveHEAD)
		{
			int difference,i;
			i=0;
			difference = active->receiveHEAD-active->readHEAD;
			for(;i<difference;i++)
			{
				if(active->receiveBuffer[active->readHEAD+i]=='\r'||active->receiveBuffer[active->readHEAD+i]=='\n')
				{
					writeDirect(&active->receiveBuffer[active->readHEAD],difference);
					if(active->receiveBuffer[active->readHEAD+i]=='\r')
					{
						MAP_UART_transmitData(EUSCI_A0_BASE,'\n');
					}
					active->readHEAD = active->receiveHEAD;
				}
				else if(active->receiveBuffer[active->readHEAD+i]==3)
				{
					exit = true;
				}
			}
		}
	}
	returntoCommand();
}
void set(int argc, char *argv[]){
	MAP_Interrupt_disableMaster();
	if(argc>2)
	{
		if(strcmp(argv[1],"name")==0)
		{
			if(strlen(argv[2])<=64)
			{
				memset(machineName,0x00,64);
				strcpy(machineName,argv[2]);
				processor=saveName;
			}
		}
		else if(strcmp(argv[1],"password")==0){
			if(strlen(argv[2])<=64){
				uint8_t *password;
				password=PASSWORD_ADDRESS;
				memset(encryptedData,0x00,64);
				int i=0;
				for(;i<active->receiveHEAD;i+=16)
				{
					MAP_AES256_encryptData(AES256_BASE, argv[2][i], encryptedData[i]);
				}
				if(!MAP_FlashCtl_programMemory(encryptedData,password, 64))
					while(1);
				password=PASSWORD_SET_FLAG_ADDRESS;
				uint8_t val = 0xA4;
				if(!MAP_FlashCtl_programMemory(&val,password, 1))
									while(1);
			}
			else{
				uint8_t prompt[] = "password is too long";
				writeDirect(prompt,strlen(prompt));
			}
		}
	}
	else{
		uint8_t prompt[] = "too few arguments";
		writeDirect(prompt,strlen(prompt));
	}
	returntoCommand();
	MAP_Interrupt_enableMaster();
}
void status(int argc, char *argv[]){
}
void quit(int argc, char *argv[]){
	MAP_Interrupt_disableMaster();
	cur_state.cur_machine_state=LOGIN;
	cur_state.cur_Login_State=PROMPT;
	machinehandler=machineLoginHandler;
	loginhandler=loginPromptHandler;
	toggleBuffer();
	powerState=PCM_LPM0_LDO_VCORE0;
	loginhandler(EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG);
	MAP_Interrupt_enableMaster();
}
void initGpio(void){
	MAP_GPIO_setAsOutputPin(GPIO_PORT_P1,GPIO_PIN0);
	MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1,GPIO_PIN4);
	MAP_GPIO_clearInterruptFlag(GPIO_PORT_P1, GPIO_PIN4);
	MAP_GPIO_enableInterrupt(GPIO_PORT_P1,GPIO_PIN4);
	MAP_Interrupt_enableInterrupt(INT_PORT1);
}
void initTimer(void){
	MAP_Timer32_initModule(TIMER32_0_BASE,TIMER32_PRESCALER_256,TIMER32_32BIT,TIMER32_PERIODIC_MODE);
	MAP_Timer32_setCount(TIMER32_0_BASE,234375);
	MAP_Timer32_registerInterrupt(TIMER32_0_INTERRUPT,lpmTimer);
	MAP_Timer32_clearInterruptFlag(TIMER32_0_BASE);
	MAP_Timer32_enableInterrupt(TIMER32_0_BASE);
	MAP_Timer32_startTimer(TIMER32_0_BASE,true);
}

void resetBuffer(buffer *buf){
	MAP_Interrupt_disableMaster();
	memset(buf->receiveBuffer,0x00,1024);
	//memset(buf->sendBuffer,(char)0,1024*sizeof(uint8_t));
	buf->receiveHEAD=0;
	buf->readHEAD=0;
	MAP_Interrupt_enableMaster();
}
void getFocus(buffer *buf){
	active = buf;
}
void displayInfo(void)
{
	uint8_t prompt[] = "Hello from MSP432. This application is a POC of a serial interface controller using FSM\r\n";
	int length = strlen(prompt);
	writeDirect(&prompt[0],length);
}
void onButtonInterrupt(void){
	uint32_t status;
	status = MAP_GPIO_getEnabledInterruptStatus(GPIO_PORT_P1);
	MAP_GPIO_clearInterruptFlag(GPIO_PORT_P1,status);
	machinehandler(status);
	loginhandler(status);
}
void encryptionInterrupt(void){
	MAP_AES256_clearErrorFlag(AES256_BASE);
	if(MAP_AES256_getErrorFlagStatus(AES256_BASE)&AES256_ERROR_OCCURRED)
	{
		while(1)
		{

		}
	}
}
void onUARTInterrupt(void){
	MAP_Interrupt_disableInterrupt(INT_T32_INT1);
	uint32_t flag = MAP_UART_getInterruptStatus(EUSCI_A0_BASE,EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG|EUSCI_A_UART_STARTBIT_INTERRUPT_FLAG);
	MAP_UART_clearInterruptFlag(EUSCI_A0_BASE,EUSCI_A_UART_RECEIVE_INTERRUPT|EUSCI_A_UART_BREAKCHAR_INTERRUPT);
	MAP_Timer32_clearInterruptFlag(TIMER32_0_BASE);
	if(cur_state.cur_machine_state!=EXECUTE)
	{
		MAP_Timer32_setCount(TIMER32_0_BASE,234475);
		MAP_Timer32_startTimer(TIMER32_0_BASE,true);
	}
	powerState=PCM_AM_LDO_VCORE0;
	machinehandler(flag);
	loginhandler(flag);
	MAP_Interrupt_enableInterrupt(INT_T32_INT1);
}
void lpmTimer(void){
	uint32_t status = MAP_Timer32_getInterruptStatus(TIMER32_0_BASE);
	MAP_Timer32_clearInterruptFlag(TIMER32_0_BASE);
	if(status>0)
	{
		switch(cur_state.cur_machine_state){
		case LOGIN:
			powerState=PCM_LPM0_LDO_VCORE0;
			break;
		case DISCONNECT:
			powerState=PCM_LPM3;
			break;
		case COMMAND:
			powerState=PCM_LPM0_LDO_VCORE1;
			break;
		}
	}
}
void saveName(){
	uint8_t *src;
	src = NAME_SET_FLAG_ADDRESS;
	uint8_t val = 0xA4;
		if(!MAP_FlashCtl_programMemory(&val,src, 1))
			                while(1);
	src = NAME_ADDRESS;
	if(!MAP_FlashCtl_programMemory(machineName,src, 64))
	                while(1);
	processor = idle;
}
void returntoCommand(void){
	toggleBuffer();
	MAP_UART_transmitData(EUSCI_A0_BASE,'\r');
	MAP_UART_transmitData(EUSCI_A0_BASE,'\n');
	writeDirect(&machineName[0],64);
	MAP_UART_transmitData(EUSCI_A0_BASE,'>');
	cur_state.cur_machine_state=COMMAND;
	machinehandler=machineCommandHandler;
	run = noapp;
}
void toggleBuffer(void){
	if(active==&pausedqueue[1])
		active--;
	else
		active++;
	resetBuffer(active);
}

