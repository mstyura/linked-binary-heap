#include "linked_binary_heap.h"

#include <inttypes.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

#if defined(LINKED_BINARY_HEAP_DEBUG)
#define LINKED_BINARY_HEAP_VERIFY_MUTATE_FUNCTIONS
#endif

#define UINT32_GT(a, b) (((b) - (a)) & 0x80000000)

#if defined(NDEBUG)
#define ASSERT_WITH_MSG(expression, msg) \
do { (void)((void) (expression), (void)(msg)); } while (0)
#else
#define ASSERT_WITH_MSG(expression, msg) \
do { assert(((void)(msg), (expression))); } while (0)
#endif


static int
linked_binary_heap_node_compare_data(
    const linked_binary_heap_node_data_comparer comparer,
    const linked_binary_heap_node_t* a,
    const linked_binary_heap_node_t* b)
{
    if (a == b)
    {
        return 0;
    }
    const int cmp = comparer(a->data, b->data);
    if (cmp != 0)
    {
        return cmp;
    }
    if (a->sequence == b->sequence)
    {
        ASSERT_WITH_MSG(0, "Only possible when compared to itself");
        return 0;
    }
    // break equal priorities by order of push into heap
    return UINT32_GT(a->sequence, b->sequence) ? 1 : -1;
}


static int
linked_binary_heap_node_verify_priorities(
    const linked_binary_heap_t* heap,
    const linked_binary_heap_node_t* node)
{
    ASSERT_WITH_MSG(heap != NULL, "Heap pointer must not be null");
    if (node == NULL)
    {
        return 0;
    }

    if (node->parent != NULL && linked_binary_heap_node_compare_data(heap->comparer, node->parent, node) > 0)
    {
        ASSERT_WITH_MSG(0, "Node's parent has bigger priority");
        return -1;
    }

    int err = 0;
    if (err == 0 && node->left != NULL)
    {
        err = linked_binary_heap_node_verify_priorities(heap, node->left);
    }
    if (err == 0 && node->right != NULL)
    {
        err = linked_binary_heap_node_verify_priorities(heap, node->right);
    }
    return err;
}


static int
linked_binary_heap_node_verify_connectivity(
    const linked_binary_heap_t* heap,
    const linked_binary_heap_node_t* node)
{
    ASSERT_WITH_MSG(heap != NULL, "Heap pointer must not be null");
    if (node == NULL)
    {
        return 0;
    }
    if (heap != node->heap)
    {
        ASSERT_WITH_MSG(0, "Node must have pointer to heap");
        return -1;
    }
    if (node == heap->root && node->parent != NULL)
    {
        ASSERT_WITH_MSG(0, "Root node must have parent set to NULL");
        return -1;
    }

    int err = 0;
    if (err == 0 && node->left != NULL)
    {
        if (node->left->parent != node)
        {
            ASSERT_WITH_MSG(0, "Left subtree has wrong pointer to parent");
            return -1;
        }
        err = linked_binary_heap_node_verify_connectivity(heap, node->left);
    }

    if (err == 0 && node->right != NULL)
    {
        if (node->right->parent != node)
        {
            ASSERT_WITH_MSG(0, "Right substree has wrong pointer to parent");
            return -1;
        }
        err = linked_binary_heap_node_verify_connectivity(heap, node->right);
    }

    return err;
}


static void
linked_binary_heap_node_swap_non_adjacent(
    linked_binary_heap_t* heap,
    linked_binary_heap_node_t* a,
    linked_binary_heap_node_t* b)
{
    ASSERT_WITH_MSG(heap != NULL, "heap pointer must not be null");
    ASSERT_WITH_MSG(a != NULL, "node pointer a must not be null");
    ASSERT_WITH_MSG(b != NULL, "node pointer b must not be null");

    if (a->heap != b->heap)
    {
        ASSERT_WITH_MSG(0, "Nodes belong to the different heaps");
        return;
    }
    if (heap != a->heap)
    {
        ASSERT_WITH_MSG(0, "Nodes does not belong to the heap");
        return;
    }
    if (a->parent == b || b->parent == a)
    {
        ASSERT_WITH_MSG(0, "Nodes are adjacent");
        return;
    }
    linked_binary_heap_node_t* const a_parent = a->parent;
    linked_binary_heap_node_t* const a_left_child = a->left;
    linked_binary_heap_node_t* const a_right_child = a->right;

    linked_binary_heap_node_t* const b_parent = b->parent;
    linked_binary_heap_node_t* const b_left_child = b->left;
    linked_binary_heap_node_t* const b_right_child = b->right;

    linked_binary_heap_node_t** a_from_parent = NULL;
    if (a_parent != NULL)
    {
        if (a_parent->left == a)
        {
            a_from_parent = &a_parent->left;
        }
        else if (a_parent->right == a)
        {
            a_from_parent = &a_parent->right;
        }
        else
        {
            ASSERT_WITH_MSG(0, "Heap inconsistency detected");
            return;
        }
    }

    linked_binary_heap_node_t** b_from_parent = NULL;
    if (b_parent != NULL)
    {
        if (b_parent->left == b)
        {
            b_from_parent = &b_parent->left;
        }
        else if (b_parent->right == b)
        {
            b_from_parent = &b_parent->right;
        }
        else
        {
            ASSERT_WITH_MSG(0, "Heap inconsistency detected");
            return;
        }
    }
    // swap
    // a
    a->left = b_left_child;
    if (b_left_child != NULL)
    {
        b_left_child->parent = a;
    }

    a->right = b_right_child;
    if (b_right_child != NULL)
    {
        b_right_child->parent = a;
    }

    a->parent = b_parent;
    if (b_from_parent != NULL)
    {
        *b_from_parent = a;
    }

    // b
    b->left = a_left_child;
    if (a_left_child != NULL)
    {
        a_left_child->parent = b;
    }

    b->right = a_right_child;
    if (a_right_child != NULL)
    {
        a_right_child->parent = b;
    }

    b->parent = a_parent;
    if (a_from_parent != NULL)
    {
        *a_from_parent = b;
    }

    // maybe update root
    if (heap->root == a)
    {
        heap->root = b;
    }
    else if (heap->root == b)
    {
        heap->root = a;
    }
#if defined(LINKED_BINARY_HEAP_VERIFY_MUTATE_FUNCTIONS)
    linked_binary_heap_node_verify_connectivity(heap, heap->root);
#endif
}


static void
linked_binary_heap_node_swap_with_parent(
    linked_binary_heap_t* heap,
    linked_binary_heap_node_t* node)
{
    ASSERT_WITH_MSG(heap != NULL, "heap pointer must not be null");
    ASSERT_WITH_MSG(node != NULL, "node pointer must not be null");

    if (heap != node->heap)
    {
        ASSERT_WITH_MSG(0, "Node does notbelong to the heap");
        return;
    }

    linked_binary_heap_node_t* const parent = node->parent;
    if (parent == NULL)
    {
        return;
    }

    const int parent_is_root = (parent == heap->root);

    // populate pointers to neighbor nodes
    linked_binary_heap_node_t* const parent_parent = parent->parent;
    linked_binary_heap_node_t* const parent_left_child = parent->left;
    linked_binary_heap_node_t* const parent_right_child = parent->right;
    linked_binary_heap_node_t* const node_left_child = node->left;
    linked_binary_heap_node_t* const node_right_child = node->right;

    linked_binary_heap_node_t** parent_parent_child = NULL;
    if (parent_parent != NULL)
    {
        if (parent_parent->left == parent)
        {
            parent_parent_child = &parent_parent->left;
        }
        else if (parent_parent->right == parent)
        {
            parent_parent_child = &parent_parent->right;
        }
        else
        {
            ASSERT_WITH_MSG(0, "Heap inconsistency detected");
            return;
        }
    }

    // updated pointers (up to 10)
    node->parent = parent_parent;
    if (parent_parent_child != NULL)
    {
        *parent_parent_child = node;
    }
    parent->parent = node;

    if (node_left_child != NULL)
    {
        node_left_child->parent = parent;
    }
    if (node_right_child != NULL)
    {
        node_right_child->parent = parent;
    }

    parent->right = node_right_child;
    parent->left = node_left_child;

    if (node == parent_left_child)
    {
        node->left = parent;
        node->right = parent_right_child;
        if (parent_right_child != NULL)
        {
            parent_right_child->parent = node;
        }
    }
    else
    {
        node->right = parent;
        node->left = parent_left_child;
        if (parent_left_child != NULL)
        {
            parent_left_child->parent = node;
        }
    }

    if (parent_is_root)
    {
        heap->root = node;
    }

#if defined(LINKED_BINARY_HEAP_VERIFY_MUTATE_FUNCTIONS)
    linked_binary_heap_node_verify_connectivity(heap, heap->root);
#endif
}


static void
linked_binary_heap_node_swap_nodes(
    linked_binary_heap_t* heap,
    linked_binary_heap_node_t* a,
    linked_binary_heap_node_t* b)
{
    ASSERT_WITH_MSG(heap != NULL, "heap pointer must not be null");
    ASSERT_WITH_MSG(a != NULL, "node pointer a must not be null");
    ASSERT_WITH_MSG(b != NULL, "node pointer b must not be null");

    if (a->heap != b->heap)
    {
        ASSERT_WITH_MSG(0, "Nodes belong to the different heaps");
        return;
    }
    if (heap != a->heap)
    {
        ASSERT_WITH_MSG(0, "Nodes does not belong to the heap");
        return;
    }
    if (a == b)
    {
        return;
    }
    if (a->parent == b)
    {
        linked_binary_heap_node_swap_with_parent(heap, a);
    }
    else if (b->parent == a)
    {
        linked_binary_heap_node_swap_with_parent(heap, b);
    }
    else
    {
        linked_binary_heap_node_swap_non_adjacent(heap, a, b);
    }
}


static void
linked_binary_heap_bubble_up(
    linked_binary_heap_t* heap,
    linked_binary_heap_node_t* node)
{
    ASSERT_WITH_MSG(heap != NULL, "heap pointer must not be null");
    ASSERT_WITH_MSG(node != NULL, "node pointer must not be null");

    const linked_binary_heap_node_data_comparer comparer = heap->comparer;
    linked_binary_heap_node_t *n = node;
    while (n->parent != NULL)
    {
        if (linked_binary_heap_node_compare_data(comparer, n, n->parent) > 0)
        {
            break;
        }
        linked_binary_heap_node_swap_with_parent(heap, n);
    }
}


static void
linked_binary_heap_bubble_down(
    linked_binary_heap_t* heap,
    linked_binary_heap_node_t* node)
{
    ASSERT_WITH_MSG(heap != NULL, "heap pointer must not be null");
    ASSERT_WITH_MSG(node != NULL, "node pointer must not be null");

    // The depth 64 mean that heap has ~2^64 nodes, which should
    // be sufficiently enough, but having depth limit might prevent
    // some infinite loop bugs in any.
    const uint32_t max_depth = sizeof(size_t) * 8;
    const linked_binary_heap_node_data_comparer comparer = heap->comparer;
    for (uint32_t depth = 0; depth < max_depth; depth++)
    {
        linked_binary_heap_node_t* smallest = node;
        if (node->left != NULL && linked_binary_heap_node_compare_data(comparer, node->left, smallest) < 0)
        {
            smallest = node->left;
        }
        if (node->right != NULL && linked_binary_heap_node_compare_data(comparer, node->right, smallest) < 0)
        {
            smallest = node->right;
        }
        if (smallest == node)
        {
            return;
        }
        linked_binary_heap_node_swap_with_parent(heap, smallest);
    }
}


static size_t
linked_binary_heap_node_parent_index(size_t index)
{
    ASSERT_WITH_MSG(index != 0, "parent of 0 node is undefined");
    return (index - 1) / 2;
}


static size_t
linked_binary_heap_node_count_descendants(const linked_binary_heap_node_t* node)
{
    if (node == NULL)
    {
        return 0;
    }
    return 1
        + linked_binary_heap_node_count_descendants(node->left)
        + linked_binary_heap_node_count_descendants(node->right);
}


static void
linked_binary_heap_node_print(linked_binary_heap_node_t *node, uint32_t space)
{
    if (node == NULL)
    {
        return;
    }
    const uint32_t indent = 10;

    space += indent;

    linked_binary_heap_node_print(node->right, space);

    printf("\n");
    for (uint32_t i = indent; i < space; i++)
    {
        printf(" ");
    }
    if (node->heap->data_visualizer != NULL)
    {
        char vis[11] = {0};
        node->heap->data_visualizer(node->data, sizeof(vis) - 1, vis);
        vis[sizeof(vis)-1] = 0;
        printf("%s\n", vis);
    }
    else
    {
        printf("%p\n", node->data);
    }
    linked_binary_heap_node_print(node->left, space);
}


void
linked_binary_heap_node_get_traverse_path_from_index(
    size_t index,
    size_t* out_path,
    uint8_t* out_depth)
{
    size_t path = 0;
    uint8_t depth = 0;
    size_t parent = index;
    while (parent != 0)
    {
        path = path << 1;
        path |= (parent % 2 == 0 ? 1 : 0);
        parent = linked_binary_heap_node_parent_index(parent);
        depth++;
    }
    *out_path = path;
    *out_depth = depth;
}


int
linked_binary_heap_get_node_by_index(
    linked_binary_heap_t* heap,
    size_t index,
    linked_binary_heap_node_t **out_parent,
    linked_binary_heap_node_t ***out_node)
{
    if (out_parent == NULL || out_node == NULL || index > heap->size)
    {
        return -1;
    }

    size_t path = 0;
    uint8_t depth = 0;
    linked_binary_heap_node_get_traverse_path_from_index(index, &path, &depth);

    linked_binary_heap_node_t *parent = NULL, **node = &heap->root;
    for (uint8_t i = 0; i < depth; i++)
    {
        parent = *node;
        if (path & (((size_t)1) << i))
        {
            node = &(*node)->right;
        }
        else
        {
            node = &(*node)->left;
        }
    }
    *out_parent = parent;
    *out_node = node;
    return 0;
}


void
linked_binary_heap_init(
    linked_binary_heap_t* heap,
    linked_binary_heap_node_data_comparer comparer,
    linked_binary_heap_node_data_visualizer data_visualizer)
{
    memset(heap, 0, sizeof(*heap));
    heap->comparer = comparer;
    heap->data_visualizer = data_visualizer;
}


void
linked_binary_heap_node_init(
    linked_binary_heap_node_t* node,
    void* data)
{
    memset(node, 0, sizeof(*node));
    node->data = data;
}


size_t
linked_binary_heap_size(
    const linked_binary_heap_t* heap)
{
    return heap->size;
}

uint32_t
linked_binary_heap_version(
    const linked_binary_heap_t* heap)
{
    return heap->mod_count;
}


int
linked_binary_heap_contains_node(
    const linked_binary_heap_t* heap,
    const linked_binary_heap_node_t* node)
{
    return heap == node->heap;
}


void
linked_binary_heap_remove(
    linked_binary_heap_t* heap,
    linked_binary_heap_node_t* node)
{
    if (heap != node->heap)
    {
        ASSERT_WITH_MSG(0, "Node belong to different heap");
        return;
    }

    heap->size -= 1;
    heap->mod_count += 1;
    if (heap->size > 0)
    {
        linked_binary_heap_node_t* parent;
        linked_binary_heap_node_t* last_node, **last_node_loc;
        int ret = linked_binary_heap_get_node_by_index(heap, heap->size, &parent, &last_node_loc);
        ASSERT_WITH_MSG(ret == 0, "Node lookup must succeed");
        last_node = *last_node_loc;

        linked_binary_heap_node_swap_nodes(heap, node, last_node);
        if (node->parent->left == node)
        {
            node->parent->left = NULL;
        }
        else if (node->parent->right == node)
        {
            node->parent->right = NULL;
        }
        else
        {
            ASSERT_WITH_MSG(0, "Wrong link from parent node");
            return;
        }

        linked_binary_heap_bubble_down(heap, last_node);
        linked_binary_heap_bubble_up(heap, last_node);
    }
    else
    {
        heap->root = NULL;
    }

    node->left = NULL;
    node->right = NULL;
    node->parent = NULL;
    node->heap = NULL;
    node->sequence = 0;
#if defined(LINKED_BINARY_HEAP_VERIFY_MUTATE_FUNCTIONS)
    linked_binary_heap_verify(heap);
#endif
}


int
linked_binary_heap_peek(
    const linked_binary_heap_t* heap,
    linked_binary_heap_node_t** out_node)
{
    ASSERT_WITH_MSG(heap != NULL, "Heap pointer must not be null");
    ASSERT_WITH_MSG(out_node != NULL, "Pointer to out node must not be null");
    if (heap->size > 0)
    {
        ASSERT_WITH_MSG(heap->root != NULL, "Heap root must be not null when size is not 0");
        *out_node = heap->root;
        return 0;
    }
    return -1;
}


int
linked_binary_heap_pop(
    linked_binary_heap_t* heap,
    linked_binary_heap_node_t** out_node)
{
    ASSERT_WITH_MSG(heap != NULL, "Heap pointer must not be null");
    ASSERT_WITH_MSG(out_node != NULL, "Pointer to out node must not be null");
    if (0 != linked_binary_heap_peek(heap, out_node))
    {
        return -1;
    }
    linked_binary_heap_remove(heap, *out_node);
    return 0;
}


void
linked_binary_heap_push(
    linked_binary_heap_t* heap,
    linked_binary_heap_node_t* node)
{
    ASSERT_WITH_MSG(heap != NULL, "Heap pointer must not be null");
    ASSERT_WITH_MSG(node != NULL, "Node pointer must not be null");
    if (node->heap != NULL)
    {
        ASSERT_WITH_MSG(0, "Node is already inserted into the heap");
        return;
    }
    linked_binary_heap_node_t* parent, **next;
    linked_binary_heap_get_node_by_index(heap, heap->size, &parent, &next);
    *next = node;
    node->heap = heap;
    node->parent = parent;
    node->sequence = heap->mod_count;
    heap->size += 1;
    heap->mod_count += 1;
    linked_binary_heap_bubble_up(heap, node);
#if defined(LINKED_BINARY_HEAP_VERIFY_MUTATE_FUNCTIONS)
    linked_binary_heap_verify(heap);
#endif
}


int
linked_binary_heap_verify(
    const linked_binary_heap_t* heap)
{
    ASSERT_WITH_MSG(heap != NULL, "Heap pointer must not be null");

    const size_t actual_nodes_count = linked_binary_heap_node_count_descendants(heap->root);
    if (actual_nodes_count != heap->size)
    {
        ASSERT_WITH_MSG(0, "Actual and declared nodes count mismatch");
        return -1;
    }

    int err = linked_binary_heap_node_verify_connectivity(heap, heap->root);
    if (err != 0)
    {
        return err;
    }
    return linked_binary_heap_node_verify_priorities(heap, heap->root);
}


void
linked_binary_heap_print(
    const linked_binary_heap_t* heap)
{
    ASSERT_WITH_MSG(heap != NULL, "Heap pointer must not be null");
    linked_binary_heap_node_print(heap->root, 0);
}
