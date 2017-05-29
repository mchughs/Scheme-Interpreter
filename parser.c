#include "value.h"
#include "linkedlist.h"
#include "tokenizer.h"
#include "talloc.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

Value* addToParseTree(Value* tree, int* depth, Value* token);
void displayValue(Value *list);
void syntaxError_1();
void syntaxError_2();
void printInput(Value *tree);
void printTree(Value *tree);

// Takes a list of tokens from a Racket program, and returns a pointer to a
// parse tree representing that program.
Value *parse(Value *tokens)
{
  Value *tree = makeNull();
  int depth = 0;

  Value *current = tokens;
  assert(current != NULL && "Error (parse): null pointer");
  while (current->type != NULL_TYPE) {
    Value *token = car(current);
    tree = addToParseTree(tree, &depth, token);
    current = cdr(current);
  }
  if (depth != 0) {
    syntaxError_2(); // too few close paren
  }
  tree = reverse(tree);
  return(tree);
}

// addToParseTree function takes in a pre-existing tree, a token to add to it,
// and a pointer to an integer depth. depth is updated to represent the number
// of unclosed open parentheses in the parse tree.
Value* addToParseTree(Value* tree, int* depth, Value* token)
{
  Value* append_cell = talloc(sizeof(Value));

  if (token->type == OPEN_TYPE) { // push

    *depth = *depth + 1; // increase depth
    append_cell->type = OPEN_TYPE;

  } else if (token->type == CLOSE_TYPE) { // pop

    *depth = *depth - 1; // decrease depth

    if (*depth < 0) {
      syntaxError_1(); // check for too many close paren
    }

    Value *new_list = makeNull(); // start a new list

    while (car(tree)->type != OPEN_TYPE) { // pop until an open paren is reached
      new_list = cons(car(tree), new_list); // append onto the new list as your go
      tree = cdr(tree);
    }

    tree = cdr(tree); // gets rid of the open paren
    append_cell = new_list; // packages who list as new item to pop onto the stack

  } else { // push
    append_cell = token;
  }

  tree = cons(append_cell, tree); // pop onto the parse
  return(tree);
}

void syntaxError_1()
{
  printf("Syntax error: too many close parentheses.\n");
  texit(EXIT_FAILURE);
  return;
}

void syntaxError_2()
{
  printf("Syntax error: not enough close parentheses.\n");
  texit(EXIT_FAILURE);
  return;
}


// Prints the tree to the screen in a readable fashion. It should look just like
// Racket code; use parentheses to indicate subtrees.
void printInput(Value *tree)
{
  if (tree->type == VOID_TYPE){
    return;
  }
  if (tree->type != CONS_TYPE) {
    displayValue(tree);
  } else {
    while (tree->type != NULL_TYPE) {
      if(car(tree)->type != CONS_TYPE) {
        displayValue(car(tree)); // display the value of the cell
      } else {
        printf("(");
        printInput(car(tree)); // recursively print from perspective of the nested list
        printf(")");
      }
      tree = cdr(tree);
    }
  }
  return;
}

void printTree(Value *tree)
{
  if (tree->type == VOID_TYPE){
    return;
  }
  if (tree->type != CONS_TYPE) {
    displayValue(tree);
  } else {
    while (tree->type != NULL_TYPE) {
      if (car(tree)->type != CONS_TYPE) {
        displayValue(car(tree)); // display the value of the cell

        if (cdr(tree)->type != NULL_TYPE && cdr(cdr(tree))->type == NULL_TYPE) {
          printf(". ");
        }
        
      } else {
        printf("(");
        printTree(car(tree)); // recursively print from perspective of the nested list
        printf(")");
      }
      tree = cdr(tree);
    }
  }
  return;
}

// prints the value in the list. Should only print ints, doubles, strs, and bools without flagging an error
void displayValue(Value *list)
{
  switch (list->type) {
  case INT_TYPE:
      printf("%i ", list->i);
      break;
  case DOUBLE_TYPE:
      printf("%f ", list->d);
      break;
  case STR_TYPE:
      printf("%s ", list->s);
      break;
  case BOOL_TYPE:
      if(list->i == 0){
        printf("#f ");
      } else {
        printf("#t ");
      }
      break;
  case SYMBOL_TYPE:
      printf("%s ", list->s);
      break;
  case OPEN_TYPE:
      printf("FLAG\n");
      printf("(");
      break;
  case CLOSE_TYPE:
      printf("FLAG\n");
      printf(")");
      break;
  case CONS_TYPE:
      printf("FLAG\n");
      display((list->c).car); //recursively display on the sublist
      display((list->c).cdr);
      break;
  case NULL_TYPE:
      printf("() ");
      break;
  case PTR_TYPE:
      printf("FLAG\n");
      printf("%p ", list->p);
      break;
  }
}
