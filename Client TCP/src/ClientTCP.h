/*
 ============================================================================
 Name        : ClientTCP.h
 Author      : Balde Omar, De Santis Samantha
 Version     :
 Copyright   : Free for educational purposes
 Description : Header file for ClientTCP
 ============================================================================
 */

#ifndef CLIENTTCP_H_
#define CLIENTTCP_H_

void clearWinSock();

void settingAddresses(struct sockaddr_in *, int, char*);
struct sockaddr_in sockBuild(int *, int, char **);

void extraSpaces(char*);
char* leadingSpaces(char*);
int argumentsValidation(int, char**);

#endif /* CLIENTTCP_H_ */
