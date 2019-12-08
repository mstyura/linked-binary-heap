#ifndef _LINKED_BINARY_HEAP_H_
#define _LINKED_BINARY_HEAP_H_

#include <inttypes.h>
#include <stddef.h>


typedef struct linked_binary_heap_node linked_binary_heap_node_t;

typedef struct linked_binary_heap linked_binary_heap_t;

/* function to compare data stored in heap nodes */
typedef int (*linked_binary_heap_node_data_comparer)(const void*, const void*);

/* function to visualize node's data as a string */
typedef void (*linked_binary_heap_node_data_visualizer)(const void*, size_t max_len, char *out_buffer);

/* structure representing heap node */
struct linked_binary_heap_node
{
    void* data; /* pointer to data associated with heap node */
    linked_binary_heap_node_t* parent; /* pointer to parent node in heap, can be null */
    linked_binary_heap_node_t* left; /* pointer to left child of node in heap, can be null */
    linked_binary_heap_node_t* right; /* pointer to right child of the heap, can be null */
    linked_binary_heap_t* heap; /* pointer to a heap containing this node */
    uint32_t sequence; /* sequence number of this node, used to resolve priority collision in push order */
};

/* structure representing heap  */
struct linked_binary_heap
{
    linked_binary_heap_node_t* root; /* pointer to root node of the heap */
    uint32_t mod_count; /* number of heap modification operations executed */
    size_t size; /* number of nodes stored in this heap */
    linked_binary_heap_node_data_comparer comparer; /* function to compare data associated with nodes */
    linked_binary_heap_node_data_visualizer data_visualizer; /* optional user-provided function to provide human readable representation of node's data */
};


void
linked_binary_heap_node_get_traverse_path_from_index(
    size_t,
    size_t*, 
    uint8_t*);


int
linked_binary_heap_get_node_by_index(
    linked_binary_heap_t*,
    size_t,
    linked_binary_heap_node_t**,
    linked_binary_heap_node_t***);


void
linked_binary_heap_init(
    linked_binary_heap_t*,
    linked_binary_heap_node_data_comparer,
    linked_binary_heap_node_data_visualizer);


void
linked_binary_heap_node_init(
    linked_binary_heap_node_t*,
    void*);


size_t
linked_binary_heap_size(
    const linked_binary_heap_t*);


uint32_t
linked_binary_heap_version(
    const linked_binary_heap_t*);


int
linked_binary_heap_contains_node(
    const linked_binary_heap_t*,
    const linked_binary_heap_node_t*);


void
linked_binary_heap_push(
    linked_binary_heap_t*,
    linked_binary_heap_node_t*);


void
linked_binary_heap_remove(
    linked_binary_heap_t*,
    linked_binary_heap_node_t*);


int
linked_binary_heap_pop(
    linked_binary_heap_t*,
    linked_binary_heap_node_t**);


int
linked_binary_heap_peek(
    const linked_binary_heap_t*,
    linked_binary_heap_node_t**);


int
linked_binary_heap_verify(
    const linked_binary_heap_t*);


void 
linked_binary_heap_print(
    const linked_binary_heap_t*);

#endif
