/*
 ============================================================================
 Name        : server.c
 Author      : Michele Pio Ardo'
 ============================================================================
 */

#if defined WIN32
#include <winsock.h>
#else
#include <netdb.h>
#define closesocket close
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "protocol.h"

// terminates use of the Winsock
void clearwinsock();

// handles errors
void errorhandler(char *errorMessage);

// returns the sum of two numbers
int add(int first_number, int second_number);

// returns the subtraction of two numbers
int sub(int first_number, int second_number);

// returns the product of two numbers
int mult(int first_number, int second_number);

// returns the division of two numbers
double division(int first_number, int second_number);

int main(void) {

#if defined WIN32
	// Initialize Winsock
	WSADATA wsa_data;
	int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
	if (result != NO_ERROR) {
		printf("Error at WSAStartup()\n");
		return 0;
	}
#endif

	int sock; // socket descriptor for the server
	struct sockaddr_in echoServAddr; // server address structure
	struct sockaddr_in echoClntAddr; // client address structure
	unsigned int cliAddrLen; // length client address
	struct hostent *remoteHost;
	// server socket creation
	// SOCKET socket(int af, int type, int protocol); return socket_id or -1
	// af: PF_INET, PF_UNIX
	// type: SOCK_STREAM (bytestream protocol for TCP), SOCK_DGRAM (datagram protocol for UDP)
	// protocol: IPPROTO_TCP (TCP protocol), IPPROTO_UDP (UDP protocol)
	sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock < 0) {
		errorhandler("socket creation failed!");
		clearwinsock();
		return -1;
	}

	// set connection settings
	memset(&echoServAddr, 0, sizeof(echoServAddr));
	echoServAddr.sin_family = AF_INET;
	echoServAddr.sin_addr.s_addr = inet_addr(ADDRESS);
	echoServAddr.sin_port = htons(PROTO_PORT);

	// binding the address to the socket
	if (bind(sock, (struct sockaddr*) &echoServAddr, sizeof(echoServAddr))
			< 0) {
		errorhandler("bind() failed!");
		closesocket(sock);
		clearwinsock();
		return -1;
	}

	while (1) {
		operation op;

		// receive operation from the client
		cliAddrLen = sizeof(echoClntAddr);
		if ((recvfrom(sock, (void*) &op, sizeof(operation), 0,
				(struct sockaddr*) &echoClntAddr, &cliAddrLen)) <= 0) {
			errorhandler(
					"recvfrom() received different number of bytes than expected!");
		}

		op.first_number = ntohl(op.first_number);
		op.second_number = ntohl(op.second_number);

		remoteHost = gethostbyaddr((char *) &echoClntAddr.sin_addr, 4, AF_INET);
		printf("Operation request '%c %d %d' from client %s, ip %s\n", op.operator, op.first_number, op.second_number, remoteHost->h_name, inet_ntoa(echoClntAddr.sin_addr));

		// performs the desired operation
		double risultato;
		switch (op.operator) {
		case '+':
			risultato = add(op.first_number, op.second_number);
			break;
		case '-':
			risultato = sub(op.first_number, op.second_number);
			break;
		case '*':
			risultato = mult(op.first_number, op.second_number);
			break;
		case '/':
			risultato = division(op.first_number, op.second_number);
			break;
		default:
			break;
		}
		char buf[BUFFER_SIZE];
		sprintf(buf, "%d %c %d = %g", op.first_number, op.operator, op.second_number, risultato);

		// sends the result of the operation performed to the client
		if (sendto(sock, buf, strlen(buf), 0, (struct sockaddr*) &echoClntAddr,
				sizeof(echoClntAddr)) != strlen(buf)) {
			errorhandler("sendto() sent different number of bytes than expected!");
		}
	}

	// close server socket
	closesocket(sock);
	clearwinsock();
	return 0;
}

// terminates use of the Winsock
void clearwinsock() {
#if defined WIN32
	WSACleanup();
#endif
}

// print error message
void errorhandler(char *errorMessage) {
	printf("%s\n", errorMessage);
}

// returns the sum of two numbers
int add(int first_number, int second_number) {
	return first_number + second_number;
}

// returns the subtraction of two numbers
int sub(int first_number, int second_number) {
	return first_number - second_number;
}

// returns the product of two numbers
int mult(int first_number, int second_number) {
	return first_number * second_number;
}

// returns the division of two numbers
double division(int first_number, int second_number) {
	return (double) first_number / second_number;
}
