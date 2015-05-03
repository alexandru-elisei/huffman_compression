#include "pqueue.h"

static struct heap_huf_node {
	uint32_t index;
	tmp_huf_node *node;
};

static struct heap {
	/* 
	 * Array of pointers to the temporary Huffman tree which is stored in 
	 * memory as an array
	 */
	struct heap_huf_node *huf_nodes;
	int size;
	int max_size;
};

/* Inserts a new element into the heap */
static enum huf_result insert(struct tmp_huf_node *c, int index);

/* Rearranges the heap from the bottom up so the heap conditions are met */
static enum huf_result rearrange_from_tail();

/* Rearranges the heap from the top down so the heap conditions are met */
static enum huf_result rearrange_from_head();

/* Prints the heap */
static enum huf_result print();

/* Creates the temporary Huffman tree in memory from the heap */
static enum huf_result create_tmp_huf (tmp_huf_node *tmp_huftree,
		uint32_t tmp_huftree_size,
		uint32_t tmp_huftree_mem);

/* Returns the parent of element at position index */
static inline int get_parent(int index);
 
/* The priority queue is stored internally as a heap */
static struct heap *h = NULL;	

/* Assigns struct functions and initializes internal variables */
enum huf_result pqueue_init(struct pqueue **pq, int n)
{
	int i;

	*pq = (struct pqueue *) malloc(sizeof(struct pqueue));
	(*pq)->insert = insert;
	(*pq)->print = print;
	(*pq)->create_tmp_huf = create_tmp_huf;

	h = (struct heap *) malloc(sizeof(struct heap));
	if (h == NULL)
		return HUF_ERROR_MEMORY_ALLOC;

	/* Allocating memory for the heap elements */
	h->max_size = n;
	h->huf_nodes = (struct tmp_huf_node **) malloc(
			h->max_size * sizeof(struct tmp_huf_node *));

	if (h->huf_nodes == NULL)
		return HUF_ERROR_MEMORY_ALLOC;

	for (i = 0; i < h->max_size; i++) {
		h->huf_nodes[i] = (struct tmp_huf_node *) malloc(
				sizeof(struct tmp_huf_node));
		if (h->huf_nodes[i] == NULL)
			return HUF_ERROR_MEMORY_ALLOC;
	}
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

	printf("\n");
	for (i = 0; i < h->size; i++)
		printf("%d: char = [%c], index = %d, freq = %d, left = %d, right = %d\n",
				i, h->huf_nodes[i].node->val,
				h->huf_nodes[i].index,
				h->huf_nodes[i].node->freq,
				h->huf_nodes[i].node->left,
				h->huf_nodes[i].node->right);
	printf("\n");

	return HUF_SUCCESS;
}

static enum huf_result insert(struct tmp_huf_node *c, int index)
{
	enum huf_result r;

	if (h == NULL)
		return HUF_ERROR_QUEUE_NOT_INITIALIZED;

	if (h->size == h->max_size)
		return HUF_ERROR_QUEUE_SIZE_EXCEEDED;

	h->huf_nodes[(h->size)++] = c;
	r = rearrange_from_tail();

	return r;
}

/* Creates the temporary Huffman tree in memory from the heap */
static enum huf_result create_tmp_huf (tmp_huf_node *tmp_huftree,
		uint32_t tmp_huftree_size,
		uint32_t tmp_huftree_mem)
{
	struct tmp_huf_node *new_node;
	struct tmp_huf_node *left_child, *right_child;

	if (h == NULL || h->size == 0)
		return HUF_ERROR_QUEUE_NOT_INITIALIZED;

	if (tmp_huftree == NULL || tmp_huftree_size == 0)
		return HUF_ERROR_INVALID_ARGUMENTS;

	left_child = h->huf_nodes[0];
	right_child = h->huf_nodes[1];

	return HUF_SUCCESS;
}

/* Rearranges the heap from the bottom up so the heap conditions are met */
static enum huf_result rearrange_from_tail()
{
	struct tmp_huf_node *tmp;
	int index, parent;

	if (h == NULL || h->size == 0)
		return HUF_ERROR_QUEUE_NOT_INITIALIZED;

	if (h->size == 1)
		return HUF_SUCCESS;

	index = h->size - 1;
	tmp = h->huf_nodes[index];

	while (index > 0) {
		parent = get_parent(index);
		if (h->huf_nodes[parent]->freq < tmp->freq) {
			break;
		} else {
			h->huf_nodes[index] = h->huf_nodes[parent];
			index = parent;
		}
	}

	h->huf_nodes[index] = tmp;

	return HUF_SUCCESS;
}

/* Returns the parent of element at position index */
static inline int get_parent(int index)
{
	if (index == 1 || index == 0)
		return 0;

	if (index % 2 == 0)
		return index / 2 - 1; 
	else
		return index / 2;
}
