#include <stdio.h>
#include <stdlib.h>
#include "value.h"

// Replacement for malloc that stores the pointers allocated. It should store
// the pointers in some kind of list; a linked list would do fine, but insert
// here whatever code you'll need to do so; don't call functions in the
// pre-existing linkedlist.h. Otherwise you'll end up with circular
// dependencies, since you're going to modify the linked list to use talloc.
Value *list = NULL; //Initializes global active-list

void *talloc(size_t size)
{
  Value *val = malloc(size); // allocate space for use
  Value *new_list = malloc(sizeof(Value)); // create a cons-cell
  new_list->type = CONS_TYPE;
  new_list->c.car = val; // point to the location allocated
  new_list->c.cdr = list; // append the global list on
  list = new_list; // reassign the pointer of the global list to look at our whole new list
  return(val); // return the address of the usable point in memory
}

// Free all pointers allocated by talloc, as well as whatever memory you
// allocated in lists to hold those pointers.
void tfree()
{
  while(list->type == CONS_TYPE && (list->c.cdr) != 0){
    // if the list point anywhere other than address 0
    free(list->c.car); // free's the useable value box
    Value *temp;
    temp = list; // save the location of the active-list's head
    list = list->c.cdr; // change the active-list to a sublist
    free(temp); // free the old head
  }
  free(list->c.car); //completes a final free
  free(list);
  list = 0; //re-initializes
}

// Replacement for the C function "exit", that consists of two lines: it calls
// tfree before calling exit. It's useful to have later on; if an error happens,
// you can exit your program, and all memory is automatically cleaned up.
void texit(int status)
{
  tfree();
  exit(status);
}
