/*
 * Priority queue, implemented as a heap
 */

#ifndef PQUEUE_H
#define PQUEUE_H

#include "common.h"

struct pqueue {
	enum huf_result (*insert) (struct tmp_huf_node *c);
	enum huf_result (*print) ();
	enum huf_result (*create_tmp_huf) (tmp_huf_node *tmp_huftree,
			uint32_t tmp_huftree_size,
			uint32_t tmp_huftree_mem);
};

/* Assigns struct functions and initializes internal variables */
enum huf_result pqueue_init(struct pqueue **pq, int n);

/* Removes struct functions and frees memory */
enum huf_result pqueue_destroy(struct pqueue **pq);

#endif	/* ifndef PQUEUE.H */
