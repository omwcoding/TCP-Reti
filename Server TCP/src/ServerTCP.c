/*
 ============================================================================
 Name        : ServerTCP.c
 Author      : Balde Omar, De Santis Samantha
 Version     :
 Copyright   : Free for educational purposes
 Description : Source file for ServerTCP
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
#include <math.h>
#include "ServerTCP.h"

#define PROTOPORT 27015 // DEFINE DEFAULT PROTOCOL PORT NUMBER
#define IP "127.0.0.1" // DEFINE DEFAULT IP NUMBER
#define ERROR_PRINT "Can't calculate.\nMust receive *operator(+,-,*,/)* *integer_value_1* *integer_value 2*"
#define QLEN 5 // QLEN "5" CAN HANDLE 6 CLIENTS IN QUEUE!

int main(int argc, char *argv[]) {
#if defined WIN32
	// INITIALIZE WINSOCK
	WSADATA wsa_data;
	int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
	if (result != NO_ERROR) {
		printf("Error at WSAStartup()\n");
		return 0;
	}
#endif
	int my_socket;
	int check = 1;
	my_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (my_socket != -1) {
		if (argumentsValidation(argc, argv)) {
			//ADDRESSING SOCKET
			struct sockaddr_in sad = sockBuild(&check, argc, argv);
			if (check) {
				if (bind(my_socket, (struct sockaddr*) &sad, sizeof(sad))
						== 0) {
					//SETTING LISTEN
					if (listen(my_socket, QLEN) == 0) {
						//ACCEPTING NEW CONNECTION
						struct sockaddr_in cad; // CLIENT ADDRESS STRUCTURE
						int client_socket; //socket descriptor for the client
						int client_len; // CLIENT ADDRESS SIZE
						client_len = sizeof(cad); // SETTING CLIENT ADDRESS SIZE

						char input[150]; // STRING RECEIVED FROM CLIENT
						char operator; // VARIABLE FOR OPERATORS (+, -, *, /)
						char first[75]; // FIRST VALUE
						char second[75]; // SECOND VALUE
						char *finalRes; // RESULT, VARIABLE SENT TO cLIENT

						int clientHandler = 0; //index to handle leaving will

						while (1) {
							printf("Waiting for a client to connect...\n");
							if ((client_socket = accept(my_socket,
									(struct sockaddr*) &cad, &client_len))
									< 0) {
								printf("accept() failed.\n");
								closesocket(client_socket);
								system("pause");
								return 0;
							}
							printf("Connection established with %s:%d\n",
									inet_ntoa(cad.sin_addr), cad.sin_port);
							clientHandler = 1;
							while (clientHandler == 1) {
								memset(input, 0, sizeof(input));
								memset(first, 0, sizeof(first));
								memset(second, 0, sizeof(second));
								if (recv(client_socket, input,
										sizeof(char[150]), 0) < 0) {
									printf("Receive failed.\n");
									closesocket(client_socket);
									clientHandler = 0;
								} else {
									//QUIT REQUEST
									if ((input[0] == '=')
											&& ((input[1] == '\0')
													|| (isspace(input[1])
															&& input[2] == '\0'))) {
										closeClientSocket(client_socket);
										clientHandler = 0;
									} else {
										operator = input[0];
										//INPUT INTEGRITY CONTROL
										if (checkOperator(operator)
												&& checkInput(input)) {
											//INPUT TOKENIZATION
											fillValues(input, first,
													second);
											//NUMERIC CONTROL
											if (checkNumbers(first, second)) {
												finalRes = calculation(operator,
														first, second);
												//SENDING RESULT
												if (send(client_socket,
														finalRes,
														sizeof(char[150]), 0)
														< 0) {
													printf(
															"Failed to send.\n");
													closesocket(client_socket);
												}
											} else {
												//SENDING ERROR BACK TO CLIENT
												if (send(client_socket,
												ERROR_PRINT, sizeof(char[150]),
														0) < 0) {
													printf(
															"Failed to send.\n");
													closesocket(client_socket);
												}
											}
										} else {
											//SENDING ERROR BACK TO CLIENT
											if (send(client_socket, ERROR_PRINT,
													sizeof(char[150]), 0) < 0) {
												printf(
														"Failed to send.\n");
												closesocket(client_socket);
											}
										}
										operator = 0;
									}
								}
							}
						}
					} else {
						printf("listen() failed.\n");
						closesocket(my_socket);
						clearWinSock();
						system("pause");
						return -1;
					}
				} else {
					printf("bind() failed.\n");
					closesocket(my_socket);
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
		printf("Socket Creation failed.\n");
		clearWinSock();
		return -1;
	}
}

void clearWinSock() {
#if defined WIN32
	WSACleanup();
#endif
}

// POPULATION OF SOCKET STRUCTURE: IP and PORT
void settingAddresses(struct sockaddr_in *sad, int port, char *ip) {
	sad->sin_addr.s_addr = inet_addr(ip);
	sad->sin_port = htons(port);
}

// INSERT ARGVS IN SOCKET STRUCTURE
// IF ARGUMENTS WERE PASSED DURING SERVER RUN
struct sockaddr_in sockBuild(int *ok, int argc, char *argv[]) {
	struct sockaddr_in sad;
	memset(&sad, 0, sizeof(sad));
	sad.sin_family = AF_INET;
	if (argc == 1) {
		settingAddresses(&sad, PROTOPORT, IP);
	} else if (argc == 2) {
		settingAddresses(&sad, PROTOPORT, argv[1]);
	} else if (argc == 3) {
		int port = atoi(argv[2]);
		if (port > 0) {
			settingAddresses(&sad, port, argv[1]);
		} else {
			printf("Bad port number.\n");
			*ok = 0;
		}
	} else {
		*ok = 0;
		memset(&sad, 0, sizeof(sad));
	}
	return sad;
}

//MATH FUNCTIONS
char* sum(int first, int second) {
	int resultInt = first + second;
	static char result[150];
	itoa(resultInt, result, 10);
	return result;
}
char* sub(int first, int second) {
	int resultInt = first - second;
	static char result[150];
	itoa(resultInt, result, 10);
	return result;
}
char* mult(int first, int second) {
	int resultInt = first * second;
	static char result[150];
	itoa(resultInt, result, 10);
	return result;
}
char* division(int first, int second) {
	static char result[150];
	// "0/0"
	if (first == 0 && second == 0) {
		strcpy(result, "Indeterminate Calculation\n");
	} else if (second == 0) { // "n/0" with n integer
		strcpy(result, "Impossible Calculation\n");
	} else {
		float resultFlt = (float) first / (float) second;
		gcvt(resultFlt, 3, result);
		int i = 0;
		while (result[i + 1] != '\0') {
			i++;
		}
		if (result[i] == '.' || result[i] == ',') {
			result[i] = ' ';
		}
	}
	return result;
}

char* calculation(int operator, char *first, char *second) {
	int firstInt = atoi(first);
	int secondInt = atoi(second);
	if (operator == '+') {
		return sum(firstInt, secondInt);
	} else if (operator == '-') {
		return sub(firstInt, secondInt);
	} else if (operator == '*') {
		return mult(firstInt, secondInt);
	} else if (operator == '/') {
		return division(firstInt, secondInt);
	}
	return 0;
}



//CHECK ON OPERATOR
int checkOperator(char operator) {
	if (operator == '+' || operator == '-' || operator == '/'
			|| operator == '*') {
		return 1;
	} else {
		return 0;
	}
}

//INPUT STRING COMPATIBLE?
int checkInput(char *input) {
	int i;
	/* IS THERE A SPACE AFTER OPERATOR?
	 * FIRST VALUE EXIST?
	 * FIRST VALUE A SPACE CHAR?
	 */
	if (isspace(input[1]) && (input[2] != '\0') && !isspace(input[2])) {
		i = 2;
		while (!isspace(input[i])) {
			i++;
		}
		i++;
		//SECOND VALUE EXIST?
		if (input[i] != '\0') {
			while (input[i] != '\0' && !isspace(input[i])
					&& isdigit(input[i]) != 0) {
				i++;
			}
			//CHECK FOR UNACCEPTABLE VALUES
			if (isspace(input[i]) && input[i + 1] != '\0') {
				//printf("Unavailable operation: there are more than two values\n");
				return 0;
			} else {
				return 1;
			}
		} else {
			//printf("Unavailable operation: no second values found.\n");
			return 0;
		}
	} else {
		//printf("Unavailable operation: no values found.\n");
		return 0;
	}
}

//VALUES ARE JUST NUMBERS?
int checkNumbers(char *first, char *second) {
	int i = 0;
	int checkDigit = 1;
	//CHECK ON FIRST VALUE
	while (first[i] != '\0') {
		if (isdigit(first[i]) == 0 || first[i] == '.' || first[i] == ',') {
			checkDigit = 0;
		}
		i++;
	}
	i = 0;
	if (checkDigit == 1) {
		//CHECK ON SECOND VALUE
		while (second[i] != '\0') {
			if (isdigit(second[i]) == 0 || second[i] == '.'
					|| second[i] == ',') {
				checkDigit = 0;
				printf("%c", second[i]);
			}
			i++;
		}
		if (checkDigit == 0) {
			//printf("Unacceptable second value.\n");
		}
	} else {
		//printf("Unacceptable first value.\n");
	}
	return checkDigit;
}

// CHECKS IF CUSTOM IP IS VALID
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

// FILLS VALUES STRINGS FROM INPUT STRING
void fillValues(char *input, char *first, char *second) {
	int i = 2;
	while (!isspace(input[i])) {
		first[i - 2] = input[i];
		i++;
	}
	i++;
	int j = 0;
	while (input[i] != '\0' && !isspace(input[i])) {
		second[j] = input[i];
		i++;
		j++;
	}
}


//CLOSES CLIENT SOCKET
void closeClientSocket(int clientSocket) {
	printf("Leaving request acquired: closing socket.\n");
	closesocket(clientSocket);
}

