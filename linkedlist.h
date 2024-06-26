#include <stdbool.h>
#include "item.h"

#ifndef LINKEDLIST_H
#define LINKEDLIST_H

// Create a new NULL_TYPE item node.
Item *makeNull();

// Create a new CONS_TYPE item node.
Item *cons(Item *newCar, Item *newCdr);

// Display the contents of the linked list to the screen in some kind of
// readable format
void display(Item *list);

// Return a new list that is the reverse of the one that is passed in.
Item *reverse(Item *list);

// Utility to make it less typing to get car item.
Item *car(Item *list);

// Utility to make it less typing to get cdr item.
Item *cdr(Item *list);

// Utility to check if pointing to a NULL_TYPE item.
bool isNull(Item *item);

// Measure length of list.
int length(Item *item);


#endif
