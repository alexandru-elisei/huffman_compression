#include <stdint.h>

#include "common.h"

#define CHECK_RESULT(r)						\
	do { 							\
		if ((r) != HUF_SUCCESS) {			\
			huf_print_result((r));			\
			if (in != NULL)				\
				fclose(in);			\
			if (out != NULL)			\
				fclose(out);			\
			exit((r));				\
		}						\
	} while (0)		

int main(int argc, char **argv)
{
	FILE *in = NULL, *out = NULL;
	uint8_t c;

	if (argc < 3)
		CHECK_RESULT(HUF_ERROR_INVALID_ARGUMENTS);

	in = fopen(argv[1], "r");
	if (in == NULL)
		CHECK_RESULT(HUF_ERROR_FILE_ACCESS);

	out = fopen(argv[2], "w");
	if (out == NULL)
		CHECK_RESULT(HUF_ERROR_FILE_ACCESS);

	return EXIT_SUCCESS;
}
