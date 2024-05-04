#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "talloc.h"
#include "linkedlist.h"

// Create a null Item
Item *makeNull() {
    Item *item = talloc(sizeof(Item));
    item->type = NULL_TYPE;
    return item;
}

// Takes in car and cdr Items, returns cons cell
Item *cons(Item *newCar, Item *newCdr) {
    Item *item = talloc(sizeof(Item));
    item->type = CONS_TYPE;
    item->c = (struct ConsCell) {newCar, newCdr};
    return item;
}

// Print out list
void display(Item *list) {
    printf("(");
    while(!isNull(list)) {
        switch(car(list)->type) {
        case INT_TYPE:
            printf("%i", car(list)->i);
            break;
        case DOUBLE_TYPE:
            printf("%lf", car(list)->d);
            break;
        case STR_TYPE:
            printf("%s", car(list)->s);
            break;
        /*case CONS_TYPE:
            display(car(list));
            break;*/
        case NULL_TYPE:
            printf("()");
            break;
        default:
            break;
        }
        if(!isNull(cdr(list))) printf(" ");
        list = cdr(list);
    }
    printf(")");
}

// Takes in list, returns new list in reverse order
Item *reverse(Item *list) {
    Item *oldList = list;
    Item *child = talloc(sizeof(Item));
    child->type = NULL_TYPE;
    while(!isNull(list)) {
        Item *newList = talloc(sizeof(Item));
        newList->type = CONS_TYPE;
        newList->c.cdr = child;

        newList->c.car = talloc(sizeof(Item));
        car(newList)->type = car(list)->type;

        itemType t = car(list)->type;
        if(t == INT_TYPE || t == BOOL_TYPE) {
            car(newList)->i = car(list)->i;
        }
        if(t == DOUBLE_TYPE) {
            car(newList)->d = car(list)->d;
        }
        if(t == STR_TYPE || t == SYMBOL_TYPE) {
            car(newList)->s = talloc(sizeof(char) * (strlen(car(list)->s) + 1));
            strcpy(car(newList)->s, car(list)->s);
        }
        
        child = newList;
        list = cdr(list);
    }
    return child;
}

// Return car of list
Item *car(Item *list) {
    assert(list->type == CONS_TYPE);
    return list->c.car;
}

// Return cdr of list
Item *cdr(Item *list) {
    assert(list->type == CONS_TYPE);
    return list->c.cdr;
}

// Returns if item is null
bool isNull(Item *item) {
    assert(item != NULL);
    return item->type == NULL_TYPE;
}

// Returns length of list
int length(Item *item) {
    int len = 0;
    while(!isNull(item)) {
        item = cdr(item);
        len++;
    }
    return len;
}
