/*
 * login.h
 *
 *  Created on: Aug 16, 2016
 *      Author: Owner
 */

#ifndef LOGIN_H_
#define LOGIN_H_
#include "global.h"
uint8_t encryptedData[64];
void promptLogin(void);
void initLogin(void);
void testLogin();
void setEncryptKey(void);
void setDecryptKey(void);
#endif /* LOGIN_H_ */
