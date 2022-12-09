/*
 ============================================================================
 Name        : client.c
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

int main(int argc, char *argv[]) {
	operation op;
	char operator;
	char space; // check space caracter
	int first_number;
	int second_number;
	char buf[BUFFER_SIZE];
	int sock; // socket descriptor for the client
	struct sockaddr_in echoServAddr; // server address structure
	struct sockaddr_in fromAddr; // server address structure
	struct in_addr *ina; // hostname address
	struct hostent *host; // information of host hostname
	struct hostent *remoteHost; // information of host fromAddr
	int fromSize;
	int respStringLen;
	char hostname[BUFFER_SIZE];
	char *token;
	int port;
	char delimiter[2] = ":";

	//check the parameters
	if (argc == 2) {
		token = strtok(argv[1], delimiter);
		strcpy(hostname, token);
		token = strtok(NULL, delimiter);
		port = atoi(token);
	} else {
		printf("Bad number of arguments\n");
		return 0;
	}

#if defined WIN32
	// Initialize Winsock
	WSADATA wsa_data;
	int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
	if (result != NO_ERROR) {
		errorhandler("WSAStartup() failed!");
		return 0;
	}
#endif

	// check hostname format
	int i;
	boolean flag = TRUE;
	for (i = 0; i < strlen(hostname); i++) {
		if (!(isalnum(hostname[i]) || hostname[i] == '-' || hostname[i] == '.'))
			flag = FALSE;
	}

	//  hostname resolution
	if (flag == TRUE) {
		host = gethostbyname(hostname);
		if (host == NULL) {
			fprintf(stderr, "gethostbyname() failed.\n");
			exit(EXIT_FAILURE);
		} else {
			ina = (struct in_addr*) host->h_addr_list[0];
		}
	} else {
		printf("Invalid hostname format");
		return 0;
	}

	// client socket creation
	// SOCKET socket(int af, int type, int protocol); return socket_id or -1
	// af: PF_INET, PF_UNIX
	// type: SOCK_STREAM (bytestream protocol for TCP), SOCK_DGRAM (datagram protocol for UDP)
	// protocol: IPPROTO_TCP (TCP protocol), IPPROTO_UDP (UDP protocol)
	sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (socket < 0) {
		errorhandler("socket creation failed!");
		closesocket(sock);
		clearwinsock();
		return -1;
	}

	// set connection settings
	memset(&echoServAddr, 0, sizeof(echoServAddr));
	echoServAddr.sin_family = AF_INET;
	echoServAddr.sin_addr = *ina;
	echoServAddr.sin_port = htons(port);

	while (1) {
		// input of string operation
		printf("Enter the math operator and numbers in this format: operator[space]first_number[space]second_number (Es:+ 26 4)\n");
		printf("Enter the '=' character to terminate the process\n");
		operator = '\0';
		space = '\0';
		first_number = 0;
		second_number = 0;
		memset(&op, 0, sizeof(op));
		memset(buf, '\0', BUFFER_SIZE);
		// input of a whitespaced string until [enter]
		while (fgets(buf, BUFFER_SIZE, stdin)) {
			// sscanf reads formatted input from a string (buf) and returns the number of variables filled
			//  %1[-+*/=] Before consumes all available consecutive whitespace characters from the input
			//				and one of the specified characters must be entered
			// %c fills the space variable
			// %d fills the first_number variable
			// %d fills the second_number variable
			int count_input = sscanf(buf, " %1[-+*/=]%c%d %d", &operator,
					&space, &first_number, &second_number);
			// count_input must be equals to 2 because when '=' is entered and [enter] is pressed,
			// space variable is filled with '\n' character
			if (operator == '=' && count_input == 2) {
				closesocket(sock);
				clearwinsock();
				return 0;
			}
			// terminate string input when format is valid
			else if (operator != '=' && space == ' ' && count_input == 4) {
				break;
			} else {
				printf("Invalid format!\n");
			}
		}
		op.operator = operator;
		op.first_number = htonl(first_number);
		op.second_number = htonl(second_number);

		// send data of operation
		if (sendto(sock, (void*) &op, sizeof(op), 0,
				(struct sockaddr*) &echoServAddr, sizeof(echoServAddr))
				!= sizeof(operation)) {
			errorhandler("sendto() sent different number of bytes than expected");
			closesocket(sock);
			clearwinsock();
			return -1;
		}

		// receive and print the result of operation
		memset(buf, '\0', BUFFER_SIZE);
		fromSize = sizeof(fromAddr);
		respStringLen = recvfrom(sock, buf, BUFFER_SIZE, 0,
				(struct sockaddr*) &fromAddr, &fromSize);

		if (echoServAddr.sin_addr.s_addr != fromAddr.sin_addr.s_addr) {
			fprintf(stderr, "Error: received a packet from unknown source.\n");
			closesocket(sock);
			clearwinsock();
			exit(EXIT_FAILURE);
		}

		if (respStringLen > 0) {
			remoteHost = gethostbyaddr((char *) &fromAddr.sin_addr, 4, AF_INET);
			printf("Received result from server %s, ip %s: %s\n\n", remoteHost->h_name, inet_ntoa(fromAddr.sin_addr), buf);
		}
		else {
			printf("No response received from the server");
			closesocket(sock);
			clearwinsock();
			return -1;
		}
	}

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
