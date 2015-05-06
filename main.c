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
		uint32_t *total, uint32_t *mem, 
		char **text, uint32_t *textmem);

void gen_char_codes(struct tmp_huf_node *tmp_huftree, 
		uint16_t tmp_huftree_size, 
		char char_codes[ASCII_SIZE][CODE_SIZE]);

void print_char_codes(char char_codes[ASCII_SIZE][CODE_SIZE]);

enum huf_result compress(FILE *out, char *text, uint32_t total,
		char char_codes[ASCII_SIZE][CODE_SIZE]);

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
	char *origtext;			/* holds the non-compressed text */
	char char_codes[ASCII_SIZE][CODE_SIZE] = {{0}};
	uint32_t origtext_mem;		/* allocated memory for the text */
	uint32_t total_chars;		/* the total number of chars */
	uint16_t tmp_huftree_size;	/* temporary Huffman tree array size */
	int i;

	/* Currently allocated memory for Huffman array */
	uint32_t tmp_huftree_mem;		

	print_char_codes(char_codes);

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

	out = fopen(argv[3], "wb");
	if (out == NULL)
		CHECK_RESULT(HUF_ERROR_FILE_ACCESS);

	origtext_mem = TEXT_SIZE;
	origtext = (char *) malloc(origtext_mem * sizeof(char));
	if (origtext == NULL)
		CHECK_RESULT(HUF_ERROR_MEMORY_ALLOC);

	r = get_input(in, &tmp_huftree, &total_chars, 
			&tmp_huftree_mem, 
			&origtext, &origtext_mem);
	tmp_huftree_size = tmp_huftree_mem;
	CHECK_RESULT(r);

	/* The priority queue size is equal to all the distinct read chars */
	r = pqueue_init(&pq, tmp_huftree_size);
	CHECK_RESULT(r);

	/* 
	 * Linking the original array of chars to the priority queue, and
	 * skipping the empty root at position 0
	 */
	for (i = 0; i < tmp_huftree_size; i++) {
		r = pq->insert(&tmp_huftree[i], i);
		CHECK_RESULT(r);
	}
	printf("after inserting original tmp_huftree array:\n");
	r = pq->print();
	CHECK_RESULT(r);

	r = pq->create_tmp_huf(&tmp_huftree, &tmp_huftree_size,
		       	&tmp_huftree_mem);
	CHECK_RESULT(r);

	printf("final temporary huffman tree:\n");
	print_tmp_huftree(tmp_huftree, tmp_huftree_size);

	gen_char_codes(tmp_huftree, tmp_huftree_size, char_codes);
	print_char_codes(char_codes);

	r = compress(out, origtext, total_chars, char_codes);
	CHECK_RESULT(r);

	return EXIT_SUCCESS;
}

enum huf_result get_input(FILE *in, struct tmp_huf_node **th, 
		uint32_t *total, uint32_t *mem, 
		char **text, uint32_t *textmem)
{
	uint32_t v[ASCII_SIZE] = {0};
	/* EOF is -1, int16_t is from -256 to 255 */
	int16_t c;		
	int i, j;

	/* Index is the character code, value is the number of occurences */
	*total = 0;
	*mem = 0;
	while ((c = fgetc(in)) != EOF) {
		if (*total == *textmem) {
			*textmem = *textmem * 2;
			*text = (char *) realloc(
					*text, *textmem * sizeof(char));
		}
		(*text)[*total] = c;

		if (v[c] == 0)
			(*mem)++;
		v[c]++;
		(*total)++;
	}
	(*text)[*total] = '\0';

	*th = (struct tmp_huf_node *) malloc(
			(*mem) * sizeof(struct tmp_huf_node));
	if (*th == NULL)
		return HUF_ERROR_MEMORY_ALLOC;

	/*
	 * Creating the Huffman array by adding all the read chars starting
	 * at index 1, after the root
	 */
	i = 0;
	for (j = 0; j < ASCII_SIZE; j++)
		if (v[j] > 0) {
			(*th)[i].freq = v[j];
			(*th)[i].val = j;
			(*th)[i].visited = NOT_VISITED;
			(*th)[i].left = NO_CHILD;
			(*th)[i].right = NO_CHILD;
			i++;
		}

	print_tmp_huftree(*th, *mem);

	return HUF_SUCCESS;
}

/* Using Depth-first search on the tmp_huftree to generate the codes */
void gen_char_codes(struct tmp_huf_node *tmp_huftree, 
		uint16_t tmp_huftree_size, 
		char char_codes[ASCII_SIZE][CODE_SIZE])
{
	/* CODE_SIZE takes into account the null terminating character */
	unsigned char codes[CODE_SIZE - 1];
	unsigned char current_character;
	int dfs_stack[CODE_SIZE - 1];
	int stack_index, tree_index, codes_index;
	int i;

	/* Visiting the root */
	tree_index = tmp_huftree_size - 1;
	stack_index = 0;
	dfs_stack[stack_index] = tree_index;
	tmp_huftree[tree_index].visited = IS_VISITED;

	codes_index = 0;
	while (stack_index >= 0) {
		if ((tmp_huftree[tree_index].left == -1 ||
			tmp_huftree[tmp_huftree[tree_index].left].visited == IS_VISITED) &&
			(tmp_huftree[tree_index].right == -1 || 
			tmp_huftree[tmp_huftree[tree_index].right].visited == IS_VISITED)) {
				/* 
				 * If we are at a leaf, we generate the
				 * character code
				 */
				if (tmp_huftree[tree_index].left == -1 &&
						tmp_huftree[tree_index].right == -1) {
					current_character = tmp_huftree[tree_index].val;
					for (i = 0; i < codes_index; i++) {
						char_codes[current_character][i] = codes[i];
						printf("%c", codes[i]);
					}
					printf("\n");
					char_codes[current_character][i] = '\0';
				}
				codes_index--;

				tree_index = dfs_stack[--stack_index];
		} else {
			/* Going left */
			if (tmp_huftree[tree_index].left != -1 &&
			tmp_huftree[tmp_huftree[tree_index].left].visited == NOT_VISITED) {
				tree_index = tmp_huftree[tree_index].left;
				tmp_huftree[tree_index].visited = IS_VISITED;
				dfs_stack[++stack_index] = tree_index;
				codes[codes_index++] = LEFT_CHILD_CODE;
			/* Going right */
			} else {
				tree_index = tmp_huftree[tree_index].right;
				tmp_huftree[tree_index].visited = IS_VISITED;
				dfs_stack[++stack_index] = tree_index;
				codes[codes_index++] = RIGHT_CHILD_CODE;
			}
		}
	}
}

void print_char_codes(char char_codes[ASCII_SIZE][CODE_SIZE])
{
	int i;

	for (i = 0; i < ASCII_SIZE; i++)
		if (char_codes[i][0] != '\0')
			printf("%c - %s\n", i, char_codes[i]);
	printf("\n");
}

enum huf_result compress(FILE *out, char *text, uint32_t total,
		char char_codes[ASCII_SIZE][CODE_SIZE])
{
	int i, j;
	uint8_t bits_calculated;
	uint8_t num_to_write;
	uint16_t code_len;
	char *current_code;

	if (text == NULL)
		return HUF_ERROR_INVALID_PARAMETER;

	bits_calculated = 0;
	num_to_write = 0;
	for (i = 0; i < total; i++) {
		current_code = char_codes[(uint8_t) text[i]];
		code_len = strlen(current_code);
		/* 
		 * Calculating the decimal number corresponding to the code, 8
		 * bits at a time
		 * */
		for (j = 0; j < code_len; j++) {
			num_to_write = num_to_write * 2 + (current_code[j] - '0');
			bits_calculated++;
			if (bits_calculated == WRITE_SIZE) {
				bits_calculated = 0;
				num_to_write = 0;
				fwrite(&num_to_write, WRITE_SIZE, 1, out);
			}
		}
	}
	/* 
	 * If I haven't written a full set of 8 bits after I compressed the
	 * message, I write zeros at the end until I get 8
	 */
	if (bits_calculated > 0) {
		for (i = bits_calculated + 1; i <= WRITE_SIZE; i++)
			num_to_write = num_to_write * 2;
		fwrite(&num_to_write, WRITE_SIZE, 1, out);
	}

	return HUF_SUCCESS;
}
