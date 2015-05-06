/*
 * Various data types and functions shared across the program
 */

#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define ASCII_SIZE		(256)
#define NO_CHILD		(-1)
#define TEXT_SIZE		(100)
#define NOT_VISITED		(0)
#define IS_VISITED		(1)
#define LEFT_CHILD_CODE		('0')
#define RIGHT_CHILD_CODE	('1')
#define WRITE_SIZE		(8)		/* bits to write at a time */
/* 
 * Maximum huftree size 2^16 - 1, max level = log2(2^16 - 1 + 1) = 16.
 * We start creating the code at level 1, so we only need 15 digits. But we
 * also need the null terminating character for the code string, therefore the
 * code is 16 characters in size.
 */
#define CODE_SIZE	(16)

/* Defined when compiling */
#ifdef DEBUG

#define DEBMSG(msg)	(printf("## %s:%d: " #msg " is \"%s\" (in %s)\n",			\
		       	__FILE__, __LINE__, (msg), __FUNCTION__))

#define DEBINFO(exp)	(printf("## %s:%d: " #exp " evaluates to %d (in %s)\n",	\
		       	__FILE__, __LINE__, (exp), __FUNCTION__))

#else

#define DEBMSG(msg)	do {} while (0)
#define DEBINFO(exp)	do {} while (0)

#endif

/* Result types for function returns */
enum huf_result {
	HUF_SUCCESS			= 1, 	
	HUF_ERROR_INVALID_PARAMETER	= 2,	/* Parameter is invalid */
	HUF_ERROR_MEMORY_ALLOC		= 3,	/* Error allocating memory */
	HUF_ERROR_INVALID_RESOURCE	= 4,	/* Resource not present */
	HUF_ERROR_INVALID_ARGUMENTS	= 5,	
	HUF_ERROR_FILE_ACCESS		= 6,	/* Cannot open file */
	HUF_ERROR_END_OF_FILE		= 7,	/* Unexpected end of file */
	HUF_ERROR_QUEUE_NOT_INITIALIZED	= 8,
	HUF_ERROR_QUEUE_SIZE_EXCEEDED	= 9,
	HUF_ERROR_UNKNOWN_OPTION	= 10,	
	HUF_ERROR_UNKNOWN_ERROR		= 99,	
};

/* Huffman tree node to be used for compression */
struct __attribute__((__packed__)) huf_node {
	unsigned char val;
	int16_t left;
	int16_t right;
};

/* Temporary Huffman node necessary for building the tree in memory */
struct __attribute__((aligned)) tmp_huf_node {
	unsigned int freq;			/* number of apparitions */
	unsigned char val;
	uint8_t visited;
	int16_t left;
	int16_t right;
};

/* Prints the messages associated with the result codes */
void huf_print_result(enum huf_result msg);

enum huf_result print_tmp_huftree(struct tmp_huf_node *huf, uint16_t size);
enum huf_result print_huftree(struct huf_node *huf, uint16_t size);

#endif	/* #ifndef COMMON_H */
