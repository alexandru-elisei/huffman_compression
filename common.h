/*
 * Various data types and functions shared across the program
 */

#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>

#define ASCII_SIZE	(256)

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
	HUF_ERROR_UNKNOWN_OPTION	= 8,	
	HUF_ERROR_UNKNOWN_ERROR		= 99,	
};

void huf_print_result(enum huf_result msg);

#endif	/* #ifndef COMMON_H */