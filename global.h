/*
 * global.h
 *
 *  Created on: Aug 16, 2016
 *      Author: Owner
 */

#ifndef GLOBAL_H_
#define GLOBAL_H_
#include "driverlib.h"
#define PASSWORD_SET_FLAG_ADDRESS 0x00020000;
#define PASSWORD_ADDRESS 0x00020040;
#define PASSWORD_LENGTH 64
#define NAME_SET_FLAG_ADDRESS 0x00021000;
#define NAME_ADDRESS 0x00021040;
#define NUM_COMMANDS 4
typedef enum  {DISCONNECT,LOGIN,COMMAND,EXECUTE}MachineState;
typedef enum  {PROMPT,TEST,IDLE}LoginState;
typedef void(*statushandler)(uint_fast8_t);
extern statushandler machinehandler,loginhandler;
typedef void(*mainhandler)(void);
typedef void(*application)(int argc,char *argv[]);
extern mainhandler processor;
extern application run;
uint8_t machineName[64];
typedef struct{
	uint8_t receiveBuffer[1024];
	int receiveHEAD;
	int readHEAD;
}buffer;
buffer *active;
typedef struct{
	int argc;
	char **argv;
} args;
args *ipc;
struct State{
LoginState cur_Login_State;
MachineState cur_machine_state;
};
struct State cur_state;
void set(int argc, char *argv[]);
void status(int argc, char *argv[]);
void quit(int argc, char *argv[]);
void echo(int argc, char *argv[]);
void saveName();
void machineDisconnectHandler(uint_fast8_t flag);
void machineLoginHandler(uint_fast8_t flag);
void machineCommandHandler(uint_fast8_t flag);
void machineExecuteHandler(uint_fast8_t flag);
void loginPromptHandler(uint_fast8_t flag);
void loginTestHandler(uint_fast8_t flag);
void loginIdleHandler(uint_fast8_t flag);
#endif /* GLOBAL_H_ */
