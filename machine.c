/*
 * machine.c
 *
 *  Created on: Aug 16, 2016
 *      Author: Owner
 */
#include "machine.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
char *commands[NUM_COMMANDS] = {"set","status","exit","repeat"};
application *functions[NUM_COMMANDS] = {set,status,quit,echo};
void machineDisconnectHandler(uint32_t flag){
	if(flag & EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG)
	{
		displayInfo();
		cur_state.cur_machine_state=LOGIN;
		machinehandler=machineLoginHandler;
		cur_state.cur_Login_State=PROMPT;
		loginhandler=loginPromptHandler;
	}
	else if(flag & GPIO_PIN4)
	{
		MAP_GPIO_toggleOutputOnPin(GPIO_PORT_P1,GPIO_PIN0);
	}
	else{

	}
}
void machineLoginHandler(uint32_t flag){
	if(flag & GPIO_PIN4)
	{
		cur_state.cur_machine_state=DISCONNECT;
		cur_state.cur_Login_State=IDLE;
		machinehandler=machineDisconnectHandler;
		loginhandler=loginIdleHandler;
		uint8_t prompt[] = "Hardware Exit Detected\r\n";
		int length = strlen(prompt);
		writeDirect(&prompt[0],length);
	}
}
void machineCommandHandler(uint32_t flag){
	if(flag & GPIO_PIN4)
		{
			cur_state.cur_machine_state=DISCONNECT;
			cur_state.cur_Login_State=IDLE;
			machinehandler=machineDisconnectHandler;
			loginhandler=loginIdleHandler;
			uint8_t prompt[] = "Hardware Exit Detected\r\n";
			int length = strlen(prompt);
			writeDirect(&prompt[0],length);
		}
	if(flag & EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG)
	{
		uint8_t data = readByteDirect();
		writeDirect(&data,1);
		if(data=='\r')
		{
			uint8_t data = '\n';
			writeDirect(&data,1);
			processCommand();
		}
		else if(data=='\n'){
			processCommand();
		}
		else if(data=='\b')
		{
			if(active->receiveHEAD>active->readHEAD)
			{
				active->receiveHEAD-=1;
				active->receiveBuffer[active->receiveHEAD]=0x00;
				MAP_UART_transmitData(EUSCI_A0_BASE,' ');
				MAP_UART_transmitData(EUSCI_A0_BASE,'\b');
			}
			else{
				MAP_UART_transmitData(EUSCI_A0_BASE,'>');
			}
		}
		else{
			active->receiveBuffer[active->receiveHEAD]=data;
			if(active->receiveHEAD<1023)
			{
				active->receiveHEAD+=1;
			}
			else{
				active->receiveHEAD=0;
			}
		}
	}
}
void machineExecuteHandler(uint32_t flag){
	if(flag & GPIO_PIN4)
			{
				cur_state.cur_machine_state=DISCONNECT;
				cur_state.cur_Login_State=IDLE;
				machinehandler=machineDisconnectHandler;
				loginhandler=loginIdleHandler;
				uint8_t prompt[] = "Hardware Exit Detected\r\n";
				int length = strlen(prompt);
				writeDirect(&prompt[0],length);
			}
	if(flag & EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG)
		{
			uint8_t data = readByteDirect();

			active->receiveBuffer[active->receiveHEAD]=data;
			if(active->receiveHEAD<1023)
			{
				active->receiveHEAD+=1;
			}
			else{
				active->receiveHEAD=0;
			}

		}
}
void initMachine(void){
	cur_state.cur_Login_State=IDLE;
	cur_state.cur_machine_state=DISCONNECT;
	machinehandler=machineDisconnectHandler;
	loginhandler=loginIdleHandler;
	resetBuffer(active);
}
void processCommand(void)
{
	int argc=0;
	char *command;
	command = strtok(&active->receiveBuffer[active->readHEAD]," ");
	int i=0;
	for(;i<NUM_COMMANDS;i++)
	{
		if(strcmp(command,commands[i])==0)
		{
			argc++;
			run=functions[i];
			cur_state.cur_machine_state = EXECUTE;
			machinehandler = machineExecuteHandler;
			break;
		}
	}
	if(argc==0)
	{
		uint8_t prompt[] = "command not recognized\r\n";
		writeDirect(prompt,sizeof(prompt));
		writeDirect(&machineName[0],strlen(&machineName[0]));
		MAP_UART_transmitData(EUSCI_A0_BASE,'>');
	}
	else{
		i=0;
		for(;i<3;i++)
		{
			if(command!=NULL){
				int length = strlen(command);
				ipc->argv[i]=command;
				command=strtok(NULL," ");
				ipc->argc++;
			}
		}
	}
	active->readHEAD=active->receiveHEAD;
}
