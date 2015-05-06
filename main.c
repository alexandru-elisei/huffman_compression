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

/* Read the compressed file */
enum huf_result get_comptext(FILE *in, FILE *out,
	       	struct huf_node **huftree, 
		uint32_t *total_chars,
		uint16_t *huftree_size,
		uint8_t **comptext);

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
	uint8_t *comptext;
	char char_codes[ASCII_SIZE][CODE_SIZE] = {{0}};
	uint32_t origtext_mem;		/* allocated memory for the text */
	uint32_t total_chars;		/* the total number of chars */
	uint16_t huftree_size;	/* temporary Huffman tree array size */
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
		huftree_size = tmp_huftree_mem;
		CHECK_RESULT(r);

		/* The priority queue size is equal to all the distinct read chars */
		r = pqueue_init(&pq, huftree_size);
		CHECK_RESULT(r);

		/* Linking the original array of chars to the priority queue */
		for (i = 0; i < huftree_size; i++) {
			r = pq->insert(&tmp_huftree[i], i);
			CHECK_RESULT(r);
		}
		//printf("after inserting original tmp_huftree array:\n");
		//r = pq->print();
		CHECK_RESULT(r);

		r = pq->gen_tmp_huf(&tmp_huftree, &huftree_size,
				&tmp_huftree_mem);
		CHECK_RESULT(r);

		/*
		printf("final temporary huffman tree:\n");
		print_tmp_huftree(tmp_huftree, huftree_size);
		*/

		/* Generating the Huffman tree to be written to disk */
		huftree = (struct huf_node *) malloc(
				huftree_size * sizeof(struct huf_node));
		if (huftree == NULL)
			CHECK_RESULT(HUF_ERROR_MEMORY_ALLOC);
		r = gen_huf(huftree, tmp_huftree, huftree_size);
		CHECK_RESULT(r);
		//print_huftree(huftree, huftree_size);
		write_huf(out, total_chars, huftree_size, huftree);

		gen_char_codes(huftree, huftree_size, char_codes);
		//print_char_codes(char_codes);

		r = compress(out, origtext, total_chars, char_codes);
		CHECK_RESULT(r);
	} else {
		r = get_comptext(in, out, &huftree, &total_chars, 
				&huftree_size, &comptext);
		CHECK_RESULT(r);

		/*
		gen_char_codes(huftree, huftree_size, char_codes);
		print_char_codes(char_codes);
		*/
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
			(*th)[i].left = NO_CHILD;
			(*th)[i].right = NO_CHILD;
			i++;
		}

	//print_tmp_huftree(*th, *mem);

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
	tree_index = huftree_size - 1;
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
					for (i = 0; i < codes_index; i++) {
						char_codes[current_character][i] = codes[i];
						//printf("%c", codes[i]);
					}
					//printf("\n");
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
	 * message, I write zeros at the end until I get 8
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
	int i;

	fwrite(&total_chars, sizeof(uint32_t), 1, out);
	fwrite(&huftree_size, sizeof(uint16_t), 1, out);

	/* Writing Huffman tree root first */
	fwrite(&(huftree[--huftree_size]), sizeof(struct huf_node), 1, out);

	/*
	printf("%d: char = %c, left = %d, right = %d\n", huftree_size,
			huftree[huftree_size].val,
			huftree[huftree_size].left, huftree[huftree_size].right);
			*/

	for (i = 0; i < huftree_size; i++) {
		fwrite(&(huftree[i]), sizeof(struct huf_node), 1, out);
		/*
		printf("%d: char = %c, left = %d, right = %d\n", i, huftree[i].val,
			huftree[i].left, huftree[i].right);
			*/
	}
}

/* Reads the compressed file */
enum huf_result get_comptext(FILE *in, FILE *out,
	       	struct huf_node **huftree, 
		uint32_t *total_chars,
		uint16_t *huftree_size,
		uint8_t **comptext)
{
	int i;
	uint8_t code_block, mask;
	int16_t bitcount;
	uint32_t chars_decompressed;

	fread(total_chars, sizeof(uint32_t), 1, in);
	fread(huftree_size, sizeof(uint16_t), 1, in);

	/*
	printf("total_chars = %d\n", *total_chars);
	printf("huftree_size = %d\n", *huftree_size);
	*/

	*huftree = (struct huf_node *) malloc(
			*huftree_size * sizeof(struct huf_node));
	/* Root is written first, stored in the last position of the huftree */
	fread(&((*huftree)[*huftree_size - 1]), sizeof(struct huf_node), 1, in);
	/* Reading the rest of the Huffman tree */
	for (i = 0; i < *huftree_size - 1; i++)
		fread(&((*huftree)[i]), sizeof(struct huf_node), 1, in);

	//print_huftree(*huftree, *huftree_size);

	i = *huftree_size - 1;
	chars_decompressed = 0;
	while (chars_decompressed < *total_chars) {
		fread(&code_block, sizeof(uint8_t), 1, in);
		for (bitcount = WRITE_SIZE - 1; bitcount >= 0; bitcount--) {
			mask = 0;
			mask = 1 << bitcount;

			/* Daca am 0 pe pozitia bitcount merg la stanga */
			if ((code_block & mask) == 0)
				i = ((*huftree)[i].left);
			else
				i = ((*huftree)[i].right);

			/* 
			 * Daca sunt la un nod frunza, il printez si incep
			 * calcularea drumului de la radacina
			 */
			if ((*huftree)[i].left == NO_CHILD && 
					(*huftree)[i].right == NO_CHILD) {
				fputc((*huftree)[i].val, out);
				chars_decompressed++;

				if (chars_decompressed == *total_chars)
					break;

				i = *huftree_size - 1;
			}
		}
	}

	return HUF_SUCCESS;
}
