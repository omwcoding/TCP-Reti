/*
 ============================================================================
 Name        : ServerTCP.h
 Author      : Balde Omar, De Santis Samantha
 Version     :
 Copyright   : Free for educational purposes
 Description : Header file for ServerTCP
 ============================================================================
 */

#ifndef SERVERTCP_H_
#define SERVERTCP_H_

void clearWinSock();

void settingAddresses(struct sockaddr_in *, int, char*);
struct sockaddr_in sockBuild(int *, int, char **);

char* sum(int, int);
char* sub(int, int);
char* mult(int, int);
char* division(int, int);
char* calculation(int, char*, char*);

int checkOperator(char);
int checkInput(char*);
int checkNumbers(char*, char*);
int argumentsValidation(int, char**);

void fillValues(char*, char*, char*);

void closeClientSocket(int);

#endif /* SERVERTCP_H_ */
