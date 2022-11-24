/*
 ============================================================================
 Name        : ClientTCP.c
 Author      : Balde Omar, De Santis Samantha
 Version     :
 Copyright   : Free for educational purposes
 Description : Source file for ClientTCP
 ============================================================================
 */

#if defined WIN32
#include <winsock.h>
#else
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#define closesocket close
#endif
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "ClientTCP.h"

#define PROTOPORT 27015 // DEFINEDEFAULT PROTOCOL PORT NUMBER
#define IP "127.0.0.1" // DEFINE DEFAULT IP

int main(int argc, char *argv[]) {

#if defined WIN32
	// WINSOCK INITIALIZATION
	WSADATA wsa_data;
	int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
	if (result != NO_ERROR) {
		printf("Error at WSAStartup()\n");
		return 0;
	}
#endif
	int var_socket;
	int check = 1;
	var_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (var_socket != -1) {
		if (argumentsValidation(argc, argv)) {
			//BUILDING SOCKET ADDRESSES
			struct sockaddr_in sad = sockBuild(&check, argc, argv);
			if (check) {
				//CHECK IF SERVER IS REACHABLE
				if (connect(var_socket, (struct sockaddr*) &sad, sizeof(sad))
						== 0) {
					printf(
							"\n\nConnection established.\nMake an operation with"
									" parameters in this order:\n*operator(+,-,*,/)* *integer_value_1*"
									"*integer_value 2*\n ex: + 23 45\nPress = to quit connection\n\n\n");
					char input[150];
					char resultant[150];
					char *rmvSpace;
					while (1) {
						memset(input, 0, sizeof(input));
						printf("\nOperation: ");
						gets(input);
						rmvSpace = leadingSpaces(input);
						//QUIT PROGRAM WITH CHARACTER "="
						if ((rmvSpace[0] == '=') && (rmvSpace[1] == '\0')) {
							send(var_socket, rmvSpace, sizeof(char[150]), 0);
							closesocket(var_socket);
							clearWinSock();
							return 1;
						} else {
							if (send(var_socket, rmvSpace, sizeof(char[150]), 0)
									!= -1) {
								//CLIENTS WAITS FOR AN ANSWER
								if (recv(var_socket, resultant, sizeof(char[150]),
										0) != -1) {
									printf("Result: %s\n", resultant);
								} else {
									printf("Failed to receive.\n");
									closesocket(var_socket);
									clearWinSock();
									return -1;
								}
							} else {
								printf("Failed to send.\n");
								closesocket(var_socket);
								clearWinSock();
								return -1;
							}
						}
						memset(rmvSpace, 0, sizeof(char[150]));
					}
				} else {
					printf("Failed to connect.\n");
					closesocket(var_socket);
					clearWinSock();
					return -1;
				}
			} else {
				printf("Socket Build failed.\n");
				clearWinSock();
				return -1;
			}
		} else {
			printf("Invalid arguments.\n");
			clearWinSock();
			return -1;
		}
	} else {
		printf("Socket creation failed.\n");
		clearWinSock();
		return -1;
	}
}



void clearWinSock() {
#if defined WIN32
	WSACleanup();
#endif
}

// FILL SOCKET STRUCTURE: IP and PORT
void settingAddresses(struct sockaddr_in *sad, int port, char *ip) {
	sad->sin_addr.s_addr = inet_addr(ip);
	sad->sin_port = htons(port);
}

// INSERT ARGVS IN SOCKET STRUCTURE
// IF ARGUMENTS WERE PASSED DURING SERVER RUN
struct sockaddr_in sockBuild(int *ok, int argc, char *argv[]) {
	struct sockaddr_in cad;
	memset(&cad, 0, sizeof(cad));
	cad.sin_family = AF_INET;
	if (argc == 1) {
		settingAddresses(&cad, PROTOPORT, IP);
	} else if (argc == 2) {
		settingAddresses(&cad, PROTOPORT, argv[1]);
	} else if (argc == 3) {
		int port = atoi(argv[2]);
		if (port > 0) {
			settingAddresses(&cad, port, argv[1]);
		} else {
			printf("Bad port number.\n");
			*ok = 0;
		}
	} else {
		*ok = 0;
		memset(&cad, 0, sizeof(cad));
	}
	return cad;
}

// TRANSFORM INPUT LIKE "+      23   45"
// INTO "+ 23 45"
void extraSpaces(char *str) {
	int i, x;
	for (i = x = 0; str[i]; ++i)
		if (!isspace(str[i]) || ((i > 0) && !isspace(str[i - 1]))) {
			str[x++] = str[i];
		}
	str[x] = '\0';
}

// TRANSFORM INPUT LIKE "      + 23 45"
// INTO "+ 23 45"
char* leadingSpaces(char *str) {
	static char str1[150];
	int count = 0, j, k;
	while (str[count] == ' ') {
		count++;
	}
	for (j = count, k = 0; str[j] != '\0'; j++, k++) {
		str1[k] = str[j];
	}
	str1[k] = '\0';
	extraSpaces(str1);
	return str1;
}

// CHECKS ON CUSTOM IP AND PORT
int argumentsValidation(int argc, char **argv) {
	if (argc > 1) {
		int pointNumber = 0;
		int i = 0;
		while (argv[1][i] |= '\0') {
			if (argv[1][i] == '.') {
				if (pointNumber == 1) {
					if (argv[1][i - 1] == '0' && argv[1][i - 2] == '.'
							&& argv[1][i + 1] == '0' && argv[1][i + 3] == '0') {
						return 0;
					}
				}
				pointNumber++;
			}
			i++;
		}
		if (pointNumber == 3) {
			if (argc == 2) {
				return 1;
			} else if (argc == 3) {
				int i = 0;
				while (argv[2][i] != '\0') {
					if (!isdigit(argv[2][i]) || argv[2][i] == '.'
							|| argv[2][i] == ',') {
						return 0;
					}
					i++;
				}
				if (atoi(argv[2]) >= 0 && atoi(argv[2]) <= 65535) {
					return 1;
				} else {
					return 0;
				}
			}
		} else {
			return 0;
		}
	} else if (argc == 1) {
		return 1;
	}
	return 0;
}
