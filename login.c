/*
 * login.c
 *
 *  Created on: Aug 16, 2016
 *      Author: Owner
 */
#include "login.h"
#include <stdbool.h>
#include <string.h>
const uint8_t cipherkey [32]={ 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c,
        0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
        0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f };
void initLogin(void)
{
	MAP_AES256_setCipherKey(AES256_BASE,cipherkey,AES256_KEYLENGTH_256BIT);
	MAP_AES256_setDecipherKey (AES256_BASE,cipherkey,AES256_KEYLENGTH_256BIT);
}
void loginPromptHandler(uint32_t flag){
	if(flag & EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG){
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
	uint8_t encrypted[64];
	memcpy(&encrypted[0],&password[0],64);
	memset(encryptedData,0x00,64);
	int i=0;
	MAP_AES256_setDecipherKey (AES256_BASE,cipherkey,AES256_KEYLENGTH_256BIT);
	for(;i<64;i+=16)
	{
		MAP_AES256_decryptData(AES256_BASE,&encrypted[i], &encryptedData[i]);
	}
	i=0;
	bool isSame=true;
	for(;i<64;i++)
	{
		if(active->receiveBuffer[active->readHEAD+i]!=encryptedData[i])
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
		memcpy(&machineName[0],src,64);
	}
	else{
		uint8_t trueval= 0xA4;
		uint8_t name[] = "msp432";
		memcpy(&machineName[0],&name[0],sizeof(name));
		processor = saveName;
	}
	writeDirect(&machineName[0],strlen(machineName));
	MAP_UART_transmitData(EUSCI_A0_BASE,'>');
}
