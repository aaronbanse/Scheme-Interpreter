#include "talloc.h"
#include <stdlib.h>

// initialize active list
Item *head = NULL;

// takes in size, returns malloc(size), and adds result of malloc to active list
void *talloc(size_t size) {
    Item *new = malloc(sizeof(Item));
    new->type=CONS_TYPE;
    new->c = (struct ConsCell) {malloc(size), head};
    head = new;
    return head->c.car;
}

// free all items in active list
void tfree() {
    while(head != NULL) {
        Item *to_free = head;
        head = head->c.cdr;
        free(to_free->c.car);
        free(to_free);
    }
}

// free and exit with status "status"
void texit(int status) {
    tfree();
    exit(status);
}

