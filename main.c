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

/* Stores the ASCII text to be compressed */
enum huf_result get_origtext(FILE *in, struct tmp_huf_node **th, 
		uint32_t *total, uint32_t *mem, 
		char **text, uint32_t *textmem);

/* Decompressing, it's so simple, there's no need for more than one function */
enum huf_result decompress(FILE *in, FILE *out);

/* Generates the codes for the caracters */
void gen_char_codes(struct huf_node *huftree, 
		uint16_t huftree_size, 
		char char_codes[ASCII_SIZE][CODE_SIZE]);

/* Generates the Huffman tree to be written to file */
enum huf_result gen_huf(struct huf_node *huftree, 
		struct tmp_huf_node *tmp_huftree, 
		uint16_t huftree_size);

/* Writes the Huffman tree to file */
void write_huf(FILE *out, uint32_t total_chars, uint16_t huftree_size,
		struct huf_node *huftree);

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
	struct huf_node *huftree;
	struct pqueue *pq;
	enum huf_result r;

	FILE *in = NULL, *out = NULL;

	char option;
	char *origtext;			/* holds the non-compressed text */
	char char_codes[ASCII_SIZE][CODE_SIZE] = {{0}};
	uint32_t origtext_mem;		/* allocated memory for the text */
	uint32_t total_chars;		/* the total number of chars */
	uint16_t huftree_size;		/* temporary Huffman tree array size */
	uint32_t tmp_huftree_mem;	/* allocated memory for tmp_huftree */		
	int i;

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

	if (option == 'c') {
		origtext_mem = TEXT_SIZE;
		origtext = (char *) malloc(origtext_mem * sizeof(char));
		if (origtext == NULL)
			CHECK_RESULT(HUF_ERROR_MEMORY_ALLOC);

		r = get_origtext(in, &tmp_huftree, &total_chars, 
				&tmp_huftree_mem, 
				&origtext, &origtext_mem);
		CHECK_RESULT(r);
		huftree_size = tmp_huftree_mem;

		/* The priority queue size is equal to all the distinct read chars */
		r = pqueue_init(&pq, huftree_size - 1);
		CHECK_RESULT(r);

		/* Linking the original array of chars to the priority queue */
		for (i = 1; i < huftree_size; i++) {
			r = pq->insert(&tmp_huftree[i], i);
			CHECK_RESULT(r);
		}

		r = pq->gen_tmp_huf(&tmp_huftree, &huftree_size,
				&tmp_huftree_mem);
		CHECK_RESULT(r);
		pqueue_destroy(&pq);

		/* Generating the Huffman tree to be written to disk */
		huftree = (struct huf_node *) malloc(
				huftree_size * sizeof(struct huf_node));
		if (huftree == NULL)
			CHECK_RESULT(HUF_ERROR_MEMORY_ALLOC);

		r = gen_huf(huftree, tmp_huftree, huftree_size);
		CHECK_RESULT(r);
		write_huf(out, total_chars, huftree_size, huftree);

		gen_char_codes(huftree, huftree_size, char_codes);
		r = compress(out, origtext, total_chars, char_codes);
		CHECK_RESULT(r);

		/* Cleaning up */
		free(origtext);
		free(tmp_huftree);
		free(huftree);
	} else {
		r = decompress(in, out);
		CHECK_RESULT(r);
	}

	fclose(in);
	fclose(out);

	return EXIT_SUCCESS;
}

enum huf_result get_origtext(FILE *in, struct tmp_huf_node **th, 
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

	(*mem)++;
	*th = (struct tmp_huf_node *) malloc(
			(*mem) * sizeof(struct tmp_huf_node));
	if (*th == NULL)
		return HUF_ERROR_MEMORY_ALLOC;

	/*
	 * Creating the Huffman array by adding all the read chars starting
	 * at index 1, after the root
	 */
	(*th)[0].freq = 0;
	(*th)[0].val = 0;
	(*th)[0].left = NO_CHILD;
	(*th)[0].right = NO_CHILD;

	i = 1;
	for (j = 0; j < ASCII_SIZE; j++)
		if (v[j] > 0) {
			(*th)[i].freq = v[j];
			(*th)[i].val = j;
			(*th)[i].left = NO_CHILD;
			(*th)[i].right = NO_CHILD;
			i++;
		}

	return HUF_SUCCESS;
}

/* Using Depth-first search on the tmp_huftree to generate the codes */
void gen_char_codes(struct huf_node *huftree, 
		uint16_t huftree_size, 
		char char_codes[ASCII_SIZE][CODE_SIZE])
{
	/* CODE_SIZE takes into account the null terminating character */
	unsigned char codes[CODE_SIZE - 1];
	unsigned char current_character;
	int dfs_stack[CODE_SIZE - 1];
	int seen_nodes[huftree_size];
	int stack_index, tree_index, codes_index;
	int i;

	for (i = 0; i < huftree_size; i++)
		seen_nodes[i] = NOT_VISITED;

	/* Visiting the root */
	tree_index = 0;
	stack_index = 0;
	dfs_stack[stack_index] = tree_index;
	seen_nodes[tree_index] = IS_VISITED;

	codes_index = 0;
	while (stack_index >= 0) {
		if ((huftree[tree_index].left == -1 ||
			seen_nodes[huftree[tree_index].left] == IS_VISITED) &&
			(huftree[tree_index].right == -1 || 
			seen_nodes[huftree[tree_index].right] == IS_VISITED)) {
				/* 
				 * If we are at a leaf, we generate the
				 * character code
				 */
				if (huftree[tree_index].left == -1 &&
						huftree[tree_index].right == -1) {
					current_character = huftree[tree_index].val;
					for (i = 0; i < codes_index; i++)
						char_codes[current_character][i] = codes[i];

					char_codes[current_character][i] = '\0';
				}
				codes_index--;

				tree_index = dfs_stack[--stack_index];
		} else {
			/* Going left */
			if (huftree[tree_index].left != -1 &&
			seen_nodes[huftree[tree_index].left] == NOT_VISITED) {
				tree_index = huftree[tree_index].left;
				codes[codes_index++] = LEFT_CHILD_CODE;
			/* Going right */
			} else {
				tree_index = huftree[tree_index].right;
				codes[codes_index++] = RIGHT_CHILD_CODE;
			}
			dfs_stack[++stack_index] = tree_index;
			seen_nodes[tree_index] = IS_VISITED;
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
	uint8_t bitcount;
	uint8_t num_to_write;
	uint16_t code_len;
	char *current_code;

	if (text == NULL)
		return HUF_ERROR_INVALID_PARAMETER;

	bitcount = 0;
	num_to_write = 0;
	for (i = 0; i < total; i++) {
		current_code = char_codes[(uint8_t) text[i]];
		code_len = strlen(current_code);
		/* 
		 * Calculating the decimal number corresponding to the code, 8
		 * bits at a time
		 * */
		for (j = 0; j < code_len; j++) {
			num_to_write = (num_to_write << 1) + (current_code[j] - '0');
			bitcount++;
			if (bitcount == WRITE_SIZE) {
				bitcount = 0;
				fwrite(&num_to_write, sizeof(uint8_t), 1, out);
				num_to_write = 0;
			}
		}
	}
	/* 
	 * If I haven't written a full set of 8 bits after I compressed the
	 * message, I write zeros at the end until writing 8 bits
	 */
	if (bitcount > 0) {
		for (i = bitcount + 1; i <= WRITE_SIZE; i++)
			num_to_write = num_to_write << 1;
		fwrite(&num_to_write, sizeof(uint8_t), 1, out);
	}

	return HUF_SUCCESS;
}

/* Generates the Huffman tree to be written to file */
enum huf_result gen_huf(struct huf_node *huftree, 
		struct tmp_huf_node *tmp_huftree, 
		uint16_t huftree_size)
{
	int i;

	if (huftree_size == 0)
		return HUF_ERROR_INVALID_RESOURCE;

	for (i = 0; i < huftree_size; i++) {
		huftree[i].val = tmp_huftree[i].val;
		huftree[i].left = tmp_huftree[i].left;
		huftree[i].right = tmp_huftree[i].right;
	}

	return HUF_SUCCESS;
}

/* Writes the Huffman tree to file */
void write_huf(FILE *out, uint32_t total_chars, uint16_t huftree_size,
		struct huf_node *huftree)
{
	fwrite(&total_chars, sizeof(uint32_t), 1, out);
	fwrite(&huftree_size, sizeof(uint16_t), 1, out);
	fwrite(huftree, sizeof(struct huf_node), huftree_size, out);
}

/* Decompressing, it's so simple, there's no need for more than one function */
enum huf_result decompress(FILE *in, FILE *out)
{
	struct huf_node *huftree;	
	uint32_t total_chars;
	uint16_t huftree_size;
	uint8_t block, mask;
	int16_t bitcount;
	uint32_t chars_decompressed;
	int i;

	fread(&total_chars, sizeof(uint32_t), 1, in);
	if (total_chars < 2)
		return HUF_ERROR_INVALID_RESOURCE;

	fread(&huftree_size, sizeof(uint16_t), 1, in);
	if (huftree_size < 2)
		return HUF_ERROR_INVALID_RESOURCE;

	/* Reading the Huffman tree */
	huftree = (struct huf_node *) malloc(
			huftree_size * sizeof(struct huf_node));
	fread(huftree, sizeof(struct huf_node), huftree_size, in);

	i = 0;
	chars_decompressed = 0;
	while (chars_decompressed < total_chars) {
		fread(&block, sizeof(uint8_t), 1, in);

		for (bitcount = WRITE_SIZE - 1; bitcount >= 0; bitcount--) {
			mask = 0;
			mask = 1 << bitcount;

			/* Daca am 0 pe pozitia curenta merg la stanga */
			if ((block & mask) == 0)
				i = (huftree[i].left);
			/* Altfel merg la dreapta */
			else
				i = (huftree[i].right);

			/* 
			 * Daca sunt la un nod frunza, il printez si incep
			 * calcularea unui nou drum de la radacina
			 */
			if (huftree[i].left == NO_CHILD && 
					huftree[i].right == NO_CHILD) {
				fputc(huftree[i].val, out);
				chars_decompressed++;

				if (chars_decompressed == total_chars)
					break;
				i = 0;
			}
		}
	}
	free(huftree);

	return HUF_SUCCESS;
}
