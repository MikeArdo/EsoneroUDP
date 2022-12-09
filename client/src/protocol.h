/*
 * protocol.h
 *
 *      Author: Michele Pio Ardo'
 */

#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#define ADDRESS "127.0.0.1"
#define PROTO_PORT 60010
#define BUFFER_SIZE 255

typedef struct {
	char operator;
	int first_number;
	int second_number;
} operation;

#endif /* PROTOCOL_H_ */
