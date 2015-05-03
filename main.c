#include <stdint.h>
#include <string.h>

#include "common.h"
#include "pqueue.h"

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

enum huf_result get_input(FILE *in, struct tmp_huf_node **th, 
		uint32_t *total, uint32_t *mem);

int main(int argc, char **argv)
{
	/*
	 * The temporary Huffman tree implemented as an array which contains 
	 * at position 0 the root of the tree, then the characters read from
	 * the file and towards the tail all the other interior nodes
	 */
	struct tmp_huf_node *tmp_huftree;
	struct pqueue *pq;
	enum huf_result r;

	FILE *in = NULL, *out = NULL;

	char option;
	uint32_t total_chars;		/* the total number of chars */
	uint32_t tmp_huftree_size;	/* temporary Huffman tree array size */
	int i;

	/* Currently allocated memory for Huffman array */
	uint32_t tmp_huftree_mem;		

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

	r = get_input(in, &tmp_huftree, &total_chars, &tmp_huftree_mem);
	tmp_huftree_size = tmp_huftree_mem;
	CHECK_RESULT(r);

	/* The priority queue size is equal to all the distinct read chars */
	r = pqueue_init(&pq, tmp_huftree_size - 1);
	CHECK_RESULT(r);

	/* 
	 * Linking the original array of chars to the priority queue, and
	 * skipping the empty root at position 0
	 */
	for (i = 1; i < tmp_huftree_size; i++) {
		r = pq->insert(&tmp_huftree[i]);
		CHECK_RESULT(r);
	}
	r = pq->print();
	CHECK_RESULT(r);

	return EXIT_SUCCESS;
}

enum huf_result get_input(FILE *in, struct tmp_huf_node **th, 
		uint32_t *total, uint32_t *mem)
{
	uint32_t v[ASCII_SIZE] = {0};
	/* EOF is -1, int16_t is from -256 to 255 */
	int16_t c;		
	int i, j;

	/* Index is the character code, value is the number of occurences */
	*total = 0;
	*mem = 0;
	while ((c = fgetc(in)) != EOF) {
		if (v[c] == 0)
			(*mem)++;
		v[c]++;
		(*total)++;
	}

	/* At position zero we have the root of the Huffman tree */
	(*mem)++;
	*th = (struct tmp_huf_node *) malloc(
			(*mem) * sizeof(struct tmp_huf_node));
	if (*th == NULL)
		return HUF_ERROR_MEMORY_ALLOC;

	/* Creating empty root at position 0 */
	(*th)[0].freq = 0;
	(*th)[0].val = '\0';
	(*th)[0].left = NO_CHILD;
	(*th)[0].right = NO_CHILD;

	/*
	 * Creating the Huffman array by adding all the read chars starting
	 * at index 1, after the root
	 */
	i = 1;
	for (j = 0; j < ASCII_SIZE; j++)
		if (v[j] > 0) {
			(*th)[i].freq = v[j];
			(*th)[i].val = j;
			(*th)[i].left = NO_CHILD;
			(*th)[i].right = NO_CHILD;
			i++;
		}

	for (i = 0; i < *mem; i++)
		printf("%d: [%c] - %d\n", i, (*th)[i].val, (*th)[i].freq);

	return HUF_SUCCESS;
}
