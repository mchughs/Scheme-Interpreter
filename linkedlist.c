#include <stdbool.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "talloc.h"

// Create a new NULL_TYPE value node.
Value *makeNull()
{
  Value *value = talloc(sizeof(Value));
  value->type = NULL_TYPE;
  return(value);
}

// Create a new CONS_TYPE value node.
Value *cons(Value *car, Value *cdr)
{
  Value *cons = talloc(sizeof(Value));
  cons->type = CONS_TYPE;
  (cons->c).car = car; //point to the same value pointed to by car
  (cons->c).cdr = cdr;
  return(cons);
}

// Display the contents of the linked list to the screen in some kind of
// readable format
void display(Value *list)
{
  switch (list->type) {
  case INT_TYPE:
      printf("%i\n", list->i);
      break;
  case DOUBLE_TYPE:
      printf("%f\n", list->d);
      break;
  case STR_TYPE:
      printf("%s\n", list->s);
      break;
  case CONS_TYPE:
      display((list->c).car); //recursively display on the sublist
      display((list->c).cdr);
      break;
  case NULL_TYPE:
      printf("null\n");
      break;
  case PTR_TYPE:
      printf("%p\n", list->p);
      break;
  case BOOL_TYPE:
      if(list->i == 0){
        printf("False\n");
      } else {
        printf("True\n");
      }
      break;
  case SYMBOL_TYPE:
      printf("%s\n", list->s);
      break;
  case OPEN_TYPE:
      printf("(\n");
      break;
  case CLOSE_TYPE:
      printf(")\n");
      break;
  }
}

// NEW
Value *reverse(Value *list)
{ // We assume that the list we wish to reverse has a head which is null.
  // And that the list was created using our cons function.
  Value *last = makeNull(); //root node
  Value *curr;
  for (;list->type == CONS_TYPE; list = list->c.cdr) //increment through the list
  {
    curr = talloc(sizeof(Value)); //allocate mem for the cons-cell
    curr->type = CONS_TYPE;
    curr->c.car = list->c.car;
    curr->c.cdr = last; //append on the previous cons-cell
    last = curr; //change the address of the "root"
  }
  return(last); //return everything
}

// Utility to make it less typing to get car value. Use assertions to make sure
// that this is a legitimate operation.
Value *car(Value *list)
{
  return((list->c).car);
}

// Utility to make it less typing to get cdr value. Use assertions to make sure
// that this is a legitimate operation.
Value *cdr(Value *list)
{
  return((list->c).cdr);
}

// Utility to check if pointing to a NULL_TYPE value. Use assertions to make sure
// that this is a legitimate operation.
bool isNull(Value *value)
{
   assert( value != NULL );
   if(value->type == NULL_TYPE){
     return true;
   } else {
     return false;
   }
}

// Measure length of list. Use assertions to make sure that this is a legitimate
// operation.
int length(Value *value)
{
  int i = 0; // start with 1
  assert( value != NULL );
  if(value->type == CONS_TYPE){
    i = length(cdr(value)) + i + 1; //every cons-cell will add 1 to the count
  }
  return i;
}
