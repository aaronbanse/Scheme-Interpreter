#ifndef ITEM_H
#define ITEM_H

typedef enum {
    INT_TYPE, DOUBLE_TYPE, STR_TYPE, CONS_TYPE, NULL_TYPE, PTR_TYPE,
    OPEN_TYPE, CLOSE_TYPE, BOOL_TYPE, SYMBOL_TYPE, OPENBRACKET_TYPE, CLOSEBRACKET_TYPE,
    DOT_TYPE, SINGLEQUOTE_TYPE,
    VOID_TYPE, CLOSURE_TYPE,
    PRIMITIVE_TYPE
} itemType;

struct Item {
    itemType type;
    union {
        int i;
        double d;
        char *s;
        void *p;
        struct ConsCell {
            struct Item *car;
            struct Item *cdr;
        } c;
        
        struct Closure {
            struct Item *paramNames;
            struct Item *functionCode;
            struct Frame *frame;
        } cl;
        
        // A primitive style function; just a pointer to it, with the right
        // signature (pf = primitive function)
        struct Item *(*pf)(struct Item *);
    };
};

typedef struct Item Item;


// A frame is a linked list of bindings, and a pointer to another frame.  A
// binding is a variable name (represented as a string), and a pointer to the
// Value it is bound to. 

struct Frame {
    struct Item *bindings;
    struct Frame *parent;
};

typedef struct Frame Frame;




#endif
