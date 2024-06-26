#include <stdlib.h>
#include "item.h"

#ifndef TALLOC_H
#define TALLOC_H

// Replacement for malloc that stores the pointers allocated so they can be freed easily later.
void *talloc(size_t size);

// Free all pointers allocated by talloc, as well as whatever memory you
// allocated in lists to hold those pointers.
void tfree();

// Replacement for the C function "exit", that consists of two lines: it calls
// tfree before calling exit.
void texit(int status);

#endif

