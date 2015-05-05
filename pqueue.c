#include "pqueue.h"

/* Similar output to strcmp: 0 is equal, < 0 if a smaller than b, > 0 otherwise */
#define FREQCMP(a, b)	(h->huf_nodes[(a)].node->freq - h->huf_nodes[(b)].node->freq)

static struct heap_huf_node {
	uint16_t index;			/* index in the tmp_huftree */
	struct tmp_huf_node *node;
};

static struct heap {
	/* 
	 * Array of pointers to the temporary Huffman tree which is stored in 
	 * memory as an array
	 */
	struct heap_huf_node *huf_nodes;
	uint16_t size;
	uint16_t max_size;
};

/* Inserts a new element into the heap */
static enum huf_result insert(struct tmp_huf_node *c, uint16_t index);

/* Rearranges the heap from the bottom up so the heap conditions are met */
static enum huf_result sift_up();

/* Rearranges the heap from the top down so the heap conditions are met */
static enum huf_result sift_down(uint16_t root);

/* Prints the heap */
static enum huf_result print();

/* Creates the temporary Huffman tree in memory from the heap */
static enum huf_result create_tmp_huf (struct tmp_huf_node **tmp_huftree,
		uint16_t *tmp_huftree_size,
		uint32_t *tmp_huftree_mem);

/* Creates a new tmp_huf_node as the parent of the two children */
static struct tmp_huf_node *create_parent();

/* The priority queue is stored internally as a heap */
static struct heap *h = NULL;	

/* Returns the parent of element at position index */
static inline int get_parent(int index)
{
	if (index == 1 || index == 0 || index == 2)
		return 0;

	if (index % 2 == 0)
		return index / 2 - 1; 
	else
		return index / 2;
}

/* Assigns struct functions and initializes internal variables */
enum huf_result pqueue_init(struct pqueue **pq, uint16_t n)
{
	*pq = (struct pqueue *) malloc(sizeof(struct pqueue));
	(*pq)->insert = insert;
	(*pq)->print = print;
	(*pq)->create_tmp_huf = create_tmp_huf;

	h = (struct heap *) malloc(sizeof(struct heap));
	if (h == NULL)
		return HUF_ERROR_MEMORY_ALLOC;

	/* Allocating memory for the heap elements */
	h->max_size = n;
	h->huf_nodes = (struct heap_huf_node *) malloc(
			h->max_size * sizeof(struct heap_huf_node));
	if (h->huf_nodes == NULL)
		return HUF_ERROR_MEMORY_ALLOC;
	h->size = 0;

	return HUF_SUCCESS;
}

/* Prints the heap */
static enum huf_result print()
{
	int i;

	if (h == NULL)
		return HUF_ERROR_QUEUE_NOT_INITIALIZED;

	if (h->size == 0)
		return HUF_ERROR_QUEUE_NOT_INITIALIZED;

	printf("\n\t\theap:\n");
	for (i = 0; i < h->size; i++)
		printf("%d: char = [%c], index = %3d, freq = %3d, left = %3d, right = %3d\n",
				i, h->huf_nodes[i].node->val,
				h->huf_nodes[i].index,
				h->huf_nodes[i].node->freq,
				h->huf_nodes[i].node->left,
				h->huf_nodes[i].node->right);
	printf("\n");

	return HUF_SUCCESS;
}

static enum huf_result insert(struct tmp_huf_node *c, uint16_t index)
{
	enum huf_result r;
	uint16_t pos;

	if (h == NULL)
		return HUF_ERROR_QUEUE_NOT_INITIALIZED;

	if (h->size == h->max_size)
		return HUF_ERROR_QUEUE_SIZE_EXCEEDED;

	pos = h->size;
	h->huf_nodes[pos].node = c;
	h->huf_nodes[pos].index = index;
	h->size++;

	r = sift_up();

	return r;
}

/* Creates the temporary Huffman tree in memory from the heap */
static enum huf_result create_tmp_huf (struct tmp_huf_node **tmp_huftree,
		uint16_t *tmp_huftree_size,
		uint32_t *tmp_huftree_mem)
{
	struct tmp_huf_node *new;
	uint16_t insert_pos;

	if (h == NULL || h->size == 0)
		return HUF_ERROR_QUEUE_NOT_INITIALIZED;

	if (tmp_huftree == NULL || tmp_huftree_size == 0)
		return HUF_ERROR_INVALID_ARGUMENTS;

	while (h->size > 1) {
		/*
		printf("create_tmp_huf, before creating parent:\n");
		print();
		printf("in create_tmp_huf(), heap[4]:\n");
		printf("val = [%c], freq = %d, index = %d\n", 
				h->huf_nodes[4].node->val,
				h->huf_nodes[4].node->freq,
				h->huf_nodes[4].index);
		*/
		/* Reallocating memory for the Huffman tree, if necessary */
		new = create_parent();
		/*
		printf("create_tmp_huf, after creating parent:\n");
		print();

		printf("in create_tmp_huf(), heap[4]:\n");
		printf("val = [%c], freq = %d, index = %d\n", 
				h->huf_nodes[4].node->val,
				h->huf_nodes[4].node->freq,
				h->huf_nodes[4].index);
		*/
		/* Reallocating memory for the Huffman tree, if necessary */
		if (*tmp_huftree_mem == *tmp_huftree_size) {
			*tmp_huftree_mem = *tmp_huftree_mem * 2;
			*tmp_huftree = (struct tmp_huf_node *) realloc(
					*tmp_huftree, 
					*tmp_huftree_mem * sizeof(struct tmp_huf_node));
		}
		/* 
		 * Inserting the new Huffman interior node at the end of the Huffman
		 * tree array
		 */
		insert_pos = *tmp_huftree_size;
		(*tmp_huftree)[insert_pos] = *new;
		(*tmp_huftree_size)++;

		/*
		printf("tmp_huftree, after new element inserted:\n");
		print_tmp_huftree(*tmp_huftree, *tmp_huftree_size);

		printf("create_tmp_huf, before inserting:\n");
		print();
		*/
		insert(&((*tmp_huftree)[insert_pos]), insert_pos);
		/*
		printf("create_tmp_huf, after inserting:\n");
		print();
		*/
	}

	return HUF_SUCCESS;
}

/* Creates a new tmp_huf_node as the parent of the two children */
static struct tmp_huf_node *create_parent()
{
	struct tmp_huf_node *new;
	struct heap_huf_node left_child, right_child;

	left_child = h->huf_nodes[0];
	right_child = h->huf_nodes[1];

		printf("in create_parent(), BEFORE sift_down(1), heap[4]:\n");
		printf("val = [%c], freq = %d, index = %d\n", 
				h->huf_nodes[4].node->val,
				h->huf_nodes[4].node->freq,
				h->huf_nodes[4].index);

	h->huf_nodes[1] = h->huf_nodes[h->size - 1];
	h->size--;
	sift_down(1);

		printf("in sift_down(), AFTER sift_down(1), heap[4]:\n");
		printf("val = [%c], freq = %d, index = %d\n", 
				h->huf_nodes[4].node->val,
				h->huf_nodes[4].node->freq,
				h->huf_nodes[4].index);
	/*
	printf("after first element removed\n");
	print();
	*/

	h->huf_nodes[0] = h->huf_nodes[h->size - 1];
	h->size--;
	sift_down(0);
	/*
	printf("after second element removed\n");
	print();
	*/

	new = (struct tmp_huf_node *) malloc(sizeof(struct tmp_huf_node));
	new->val = 0;
	new->freq = left_child.node->freq + right_child.node->freq;
	new->left = left_child.index;
	new->right = right_child.index;

	return new;
}

/* Rearranges the heap from the bottom up so the heap conditions are met */
static enum huf_result sift_up()
{
	struct heap_huf_node tmp;
	int index, parent;

	if (h == NULL || h->size == 0)
		return HUF_ERROR_QUEUE_NOT_INITIALIZED;

	if (h->size == 1)
		return HUF_SUCCESS;

	index = h->size - 1;
	parent = get_parent(index);
	tmp = h->huf_nodes[index];

	printf("sift_up(), index = %d\n", index);
	printf("sift_up(), freq[index] = %d\n", h->huf_nodes[index].node->freq);
	printf("sift_up(), parent = %d\n", parent);
	printf("sift_up(), freq[parent] = %d\n", h->huf_nodes[parent].node->freq);
	/* Moving the last element up the heap until heap conditions are met */
	while (index > 0 && h->huf_nodes[parent].node->freq > tmp.node->freq) {
		h->huf_nodes[index] = h->huf_nodes[parent];
		index = parent;
		parent = get_parent(index);
	}
	h->huf_nodes[index] = tmp;

	return HUF_SUCCESS;
}

/* Rearranges the heap from the top down so the heap conditions are met */
static enum huf_result sift_down(uint16_t root)
{
	struct heap_huf_node tmp;
	int index, left_child, right_child;
	int last_index;
	int min;

	if (h == NULL || h->size == 0)
		return HUF_ERROR_QUEUE_NOT_INITIALIZED;

	if (h->size == 1)
		return HUF_SUCCESS;

	/* Moving the root downwards */
	tmp = h->huf_nodes[root];
	index = root;
	last_index = h->size - 1;
	printf("\n\nsift_down, with root %d, heap before is:\n", root);
	print();
	/* While we have a left child */
	while (2 * index < last_index) {
		left_child = 2 * index + 1;
		right_child = 2 * index + 2;
		DEBINFO(index);
		DEBINFO(left_child);
		DEBINFO(right_child);
		/* If we have both children we find out which child is smaller */
		if (right_child <= last_index) {
			if (h->huf_nodes[left_child].node->freq <= h->huf_nodes[right_child].node->freq)
				min = left_child;
			else
				min = right_child;
		/* We only have a left child */
		} else {
			min = left_child;
		}

		DEBINFO(min);
		DEBINFO(h->huf_nodes[min].node->freq);
		/* Root is smaller than both children */
		if (h->huf_nodes[min].node->freq >= tmp.node->freq) {
			break;
		} else {
			h->huf_nodes[index] = h->huf_nodes[min];
			index = min;
		}
	}
	h->huf_nodes[index] = tmp;
	printf("\n\nsift_down, with root %d, heap after is:\n", root);
	print();

	return HUF_SUCCESS;
}
		
