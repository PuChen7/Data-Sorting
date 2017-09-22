/*****
*
*	Define structures and function prototypes for your sorter
*
*
*
******/

//Suggestion: define a struct that mirrors a record (row) of the data set
typedef struct _node {
    char *name;
    char *desc;
    struct _node *next;
} node;

//Suggestion: prototype a mergesort function
