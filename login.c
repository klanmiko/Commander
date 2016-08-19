/*
 * login.c
 *
 *  Created on: Aug 16, 2016
 *      Author: Owner
 */
#include "login.h"
#include <stdbool.h>
const uint8_t cipherkey [32]={0xAA,0xBB,0xCC,0xAB,
			0xAC,0xAD,0xBC,0xDD,
			0x01,0x05,0x92,0xA2,
			0xB2,0xE5,0x61,0xD2,
			0x12,0x21,0x00,0xC5,
			0x87,0xA4,0xAE,0x06,
			0x02,0xCC,0xCD,0x93,
			0x1C,0x4B,0x2A,0xA6};
void initLogin(void)
{
	MAP_AES256_setCipherKey(AES256_BASE,cipherkey,AES256_KEYLENGTH_256BIT);
}
void loginPromptHandler(uint32_t flag){
	if(flag & EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG){
		initLogin();
		uint8_t *isPasswordSet;
			isPasswordSet = PASSWORD_SET_FLAG_ADDRESS;
			if(*isPasswordSet==0xA4)
			{
				uint8_t prompt[] = "password:";
				int length = strlen(prompt);
				writeDirect(&prompt[0],length);
				cur_state.cur_Login_State=TEST;
				loginhandler=loginTestHandler;
			}
			else{
				uint8_t prompt[] = "no password set, logging in anyways";
				int length = strlen(prompt);
				writeDirect(&prompt[0],length);
				cur_state.cur_machine_state=COMMAND;
				machinehandler=machineCommandHandler;
				cur_state.cur_Login_State=IDLE;
				loginhandler=loginIdleHandler;
				writeName();
			}
	}
}
void loginTestHandler(uint32_t flag){
	if(flag&EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG)
	{
		uint8_t x = readByteDirect();
		if(x=='\r'||x=='\n'){
			testLogin();
		}
		else if(x=='\b')
		{
			active->receiveHEAD-=1;
			active->receiveBuffer[active->receiveHEAD]=0x00;
		}
		else{
			active->receiveBuffer[active->receiveHEAD]=x;
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
void loginIdleHandler(uint32_t flag){

}
void testLogin(){
	uint8_t *password;
	password=PASSWORD_ADDRESS;
	memset(encryptedData,0x00,64);
	int i=0;
	for(;i<active->receiveHEAD;i+=16)
	{
		MAP_AES256_encryptData(AES256_BASE, active->receiveBuffer[i], encryptedData[i]);
	}
	i=0;
	bool isSame=true;
	for(;i<64;i++)
	{
		if(password[i]!=encryptedData[i])
		{
			isSame=false;
		}
	}
	if(isSame)
	{
		cur_state.cur_machine_state=COMMAND;
		machinehandler=machineCommandHandler;
		cur_state.cur_Login_State=IDLE;
		loginhandler=loginIdleHandler;
		writeName();
	}
	else{
		uint8_t prompt[] = "incorrect password\r\n";
		int length = strlen(prompt);
		writeDirect(&prompt[0],length);
		uint8_t prompt2[] = "password:";
		length = strlen(prompt2);
		writeDirect(&prompt2[0],length);
	}
	active->readHEAD=active->receiveHEAD;
}
void writeName(void){
	MAP_UART_transmitData(EUSCI_A0_BASE,'\r');
	MAP_UART_transmitData(EUSCI_A0_BASE,'\n');
	uint8_t *isNameSet;
	uint8_t *src;
	src = NAME_ADDRESS;
	isNameSet = NAME_SET_FLAG_ADDRESS;
	uint8_t val = *isNameSet;
	if(val==0xA4)
	{
		memcpy(&machineName[0],src,sizeof(machineName));
	}
	else{
		uint8_t trueval= 0xA4;
		uint8_t name[] = "msp432";
		memcpy(&machineName[0],&name[0],sizeof(name));
		processor = saveName;
	}
	writeDirect(&machineName[0],64);
	MAP_UART_transmitData(EUSCI_A0_BASE,'>');
}
