
/*
File: epan/proto.h
This struct represents the Component, which is the base structure that defines the unified interface and the pointers for hierarchical relationships.
*/

typedef struct _proto_node
{
    struct _proto_node *first_child; /* Pointer to the first nested node (child) */
    struct _proto_node *last_child;  /* Pointer to the last nested node (child) */
    struct _proto_node *next;        /* Pointer to the next sibling node (same level) */
    struct _proto_node *parent;      /* Pointer to the parent node */
    const header_field_info *hfinfo;
    field_info *finfo; /* Actual data of the decoded field */
    tree_data_t *tree_data;
} proto_node;

/*
File: epan/proto.h
Composite and Leaf roles are implemented using aliases (typedef). Since they refer to the exact same base structure, the client can treat them uniformly.
*/
typedef struct _proto_node proto_node;
typedef proto_node proto_item; /* Leaf Role: Terminal single field (without children) */
typedef proto_node proto_tree; /* Composite Role: Protocol container (capable of having children) */

/*
File: epan/proto.c
This function allows adding elements without the need to distinguish whether the passed tree is the main root or a deeply nested subtree.
*/
proto_item *
proto_tree_add_item(proto_tree *tree, int hfindex, tvbuff_t *tvb,
                    const int start, int length, const unsigned encoding)
{

    /* ... */

    return proto_tree_add_item_new(tree, hfinfo, tvb, start, length, encoding);
}