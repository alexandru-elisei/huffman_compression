#include "common.h"

#define PRINTERR(msg)	fprintf(stderr, "[ ERROR ] %s", msg)

/* Prints the message associated with the defined result codes */
void huf_print_result(enum huf_result msg)
{
	/* I don't like switch */
	if (msg == HUF_SUCCESS)
		fprintf(stderr, "[  OK!  ] Operation executed successfully.\n");
	else if (msg == HUF_ERROR_INVALID_PARAMETER)
		PRINTERR("Invalid parameter supplied.\n");
	else if (msg == HUF_ERROR_MEMORY_ALLOC)
		PRINTERR("Error while allocating memory.\n");
	else if (msg == HUF_ERROR_INVALID_RESOURCE)
		PRINTERR("Accessing invalid resource.\n");
	else if (msg == HUF_ERROR_INVALID_ARGUMENTS)
		PRINTERR("Invalid arguments.\n");
	else if (msg == HUF_ERROR_FILE_ACCESS)
		PRINTERR("Cannot access file.\n");
	else if (msg == HUF_ERROR_END_OF_FILE)
		PRINTERR("Unexpected end of file.\n");
	else if (msg == HUF_ERROR_UNKNOWN_OPTION)
		PRINTERR("Unknown option.\n");
	else if (msg == HUF_ERROR_UNKNOWN_ERROR)
		PRINTERR("Unknown error occured.\n");
}
