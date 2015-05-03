#include <stdint.h>
#include <string.h>

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

struct __attribute__((__packed__)) huf_node {
	unsigned char value;
	int16_t left;
	int16_t right;
};

struct __attribute__((aligned)) input_char {
	unsigned char val;
	unsigned int freq;	/* number of apparitions */
};

enum huf_result get_input(FILE *in, struct input_char **chars, int *n,
	       	int *separate);

int main(int argc, char **argv)
{
	struct input_char *chars;
	enum huf_result r;
	int separate_chars;
	int total_chars;

	FILE *in = NULL, *out = NULL;
	char option;

	if (argc < 4)
		CHECK_RESULT(HUF_ERROR_INVALID_ARGUMENTS);

	if (strcmp(argv[1], "-c") == 0 || strcmp(argv[1], "-C") == 0)
		option = 'c';
	else if (strcmp(argv[1], "-d") == 0 || strcmp(argv[1], "-D") == 0)
		option = 'd';
	else
		CHECK_RESULT(HUF_ERROR_UNKNOWN_OPTION);

	in = fopen(argv[2], "r");
	if (in == NULL)
		CHECK_RESULT(HUF_ERROR_FILE_ACCESS);

	out = fopen(argv[3], "w");
	if (out == NULL)
		CHECK_RESULT(HUF_ERROR_FILE_ACCESS);

	r = get_input(in, &chars, &total_chars, &separate_chars);
	CHECK_RESULT(r);

	return EXIT_SUCCESS;
}

enum huf_result get_input(FILE *in, struct input_char **chars, int *n,
		int *separate)
{
	uint32_t v[ASCII_SIZE] = {0};
	int16_t c;		/* EOF is -1, int16_t is from -256 to 255 */
	int i, j;

	*n = 0;
	*separate = 0;
	while ((c = fgetc(in)) != EOF) {
		if (v[c] == 0)
			(*separate)++;
		v[c]++;
		(*n)++;
	}

	*chars = NULL;
	*chars = (struct input_char *) malloc((*n) * sizeof(struct input_char));
	if (*chars == NULL)
		return HUF_ERROR_MEMORY_ALLOC;

	i = 0;
	for (j = 0; j < ASCII_SIZE; j++)
		if (v[j] > 0) {
			((*chars)[i]).freq = v[j];
			((*chars)[i]).val = j;
			i++;
		}

	for (i = 0; i < *separate; i++)
		printf("[%c] - %d\n", ((*chars)[i]).val, ((*chars)[i]).freq);

	return HUF_SUCCESS;
}
