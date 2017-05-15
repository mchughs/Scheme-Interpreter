#include "value.h"
#include "linkedlist.h"
#include "tokenizer.h"
#include "talloc.h"
#include "parser.h"
#include "interpreter.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

// function prototypes
Value* lookUpSymbol(Value* tree, Frame* frame);
Value* evalIf(Value* args, Frame* frame);
Value* evalLet(Value* args, Frame* frame);
void evaluationError();

void interpret(Value *tree)
{ // sets up global frame
  Frame* frame = talloc(sizeof(Frame));
  frame->bindings = NULL;
  frame->parent = NULL;

  printTree(tree); // Prints parse tree for comparison
  printf("--> ");

  Value* evaluated_tree = talloc(sizeof(Value));
  // Increments through and evaluates every S-exp
  while (tree->type != NULL_TYPE) {
    evaluated_tree = eval(car(tree), frame);
    printTree(evaluated_tree);
    tree = cdr(tree);
  }

  printf("\n"); // prints newline to maintain unix file standard

  return;
}

Value *eval(Value *tree, Frame *frame)
{  // Ints, Doubles, Bools, Nulls, and Strs all evaluate to themselves
   switch (tree->type)  {
     case INT_TYPE: {
        return(tree);
        break;
     }
     case DOUBLE_TYPE: {
        return(tree);
        break;
     }
     case STR_TYPE: {
        return(tree);
        break;
     }
     case BOOL_TYPE: {
        return(tree);
        break;
     }
     case NULL_TYPE: {
        return(tree);
        break;
     }
     case OPEN_TYPE: {
        printf("Unexpected type seen in eval\n");
        evaluationError();
        break;
     }
     case CLOSE_TYPE: {
       printf("Unexpected type seen in eval\n");
       evaluationError();
        break;
     }
     case PTR_TYPE: {
       printf("Unexpected type seen in eval\n");
       evaluationError();
        break;
     }
     case SYMBOL_TYPE: {
        return lookUpSymbol(tree, frame); // call helper
        break;
     }
     case CONS_TYPE: {
        Value *first = car(tree);
        Value *args = cdr(tree);

        // handles the car being a non-symbol type. Not necessarily useful for
        // evaluating just if and let but may be useful later
        /*
        if (first->type != SYMBOL_TYPE && first->type != CONS_TYPE) {
          Value* tree1 = talloc(sizeof(Value));
          Value* tree2 = talloc(sizeof(Value));
          tree1 = eval(first, frame);
          tree2 = eval(args, frame);
          tree = cons(tree1,tree2);
          break;
        }
        */

        if (!strcmp(first->s,"if")) {
            Value* result = evalIf(args,frame); // call helper
            return(result);
        }

        if (!strcmp(first->s,"let")) {
            Value* result = evalLet(args,frame); // call helper
            return(result);
        }

        if (car(car(first)) != NULL) { // handles cons-cells maping to cons-cells
          Value* tree1 = talloc(sizeof(Value));
          Value* tree2 = talloc(sizeof(Value));
          tree1 = eval(car(tree), frame);
          tree2 = eval(cdr(tree), frame);
          tree = cons(tree1,tree2);
        }

        // .. other special forms here...

        else {
            printf("Not a special form.\n");
           // not a recognized special form
           evaluationError();
        }
        break;
    }
  }
  return(tree);
}


// recursively check through the frame for the variable name
Value* lookUpSymbol(Value* tree, Frame* frame)
{
  char* symbol = tree->s;

  while (frame->parent != NULL) { // increment through the frames
    Value* temp_bindings = talloc(sizeof(Value));
    temp_bindings = frame->bindings;

    while (temp_bindings->type != NULL_TYPE) { // increment through the list of bindings

      Value* val = talloc(sizeof(Value));
      char* var = talloc(sizeof(char));
      Value* var_val = talloc(sizeof(Value));

      var_val = car(temp_bindings);
      var = car(var_val)->s; // variable name
      val = cdr(var_val); // value associated with variable

      if (!strcmp(symbol, var)) { // if the symbol is the same as the var
        tree = val;
        return(tree); // return the variable's associated value
      }

      temp_bindings = cdr(temp_bindings);
    }
    frame = frame->parent;
  }

  printf("Variable unassigned.\n"); // If the symbol is never found throw error
  evaluationError();
  return(tree);
}


Value* evalLet(Value* args, Frame* frame)
{
  Frame* new_frame = talloc(sizeof(Frame));
  new_frame->parent = frame; // parent is the passed in frame.

  Value* bindings = talloc(sizeof(Value));
  bindings = car(args); // according to let structure

  Value* body = talloc(sizeof(Value));
  body = car(cdr(args));

  Value* bindingsList = talloc(sizeof(Value));
  bindingsList = makeNull(); // head of linked list of bindings in the new_frame

  while (bindings->type != NULL_TYPE) {

    Value* val = talloc(sizeof(Value));
    Value* expr = talloc(sizeof(Value));
    Value* var = talloc(sizeof(Value));
    Value* var_val = talloc(sizeof(Value));

    expr = car(cdr(car(bindings))); // that's just what it is

    if (car(car(bindings))->type == SYMBOL_TYPE) {
      var = car(car(bindings));
    } else {
      printf("Let variable not a symbol.\n");
      evaluationError();
    }

    val = eval(expr ,frame);

    var_val = cons(var,val); // car = symbol type, cdr = w/e type

    bindingsList = cons(var_val, bindingsList); // add on a symbol-value pair to linked list
    bindings = cdr(bindings);
  }

  new_frame->bindings = reverse(bindingsList);

  Value* tree = talloc(sizeof(Value));
  tree = eval(body, new_frame);

  return(tree); // return the evaluation of the body given the new_frame
}


Value* evalIf(Value* args, Frame* frame)
{
  Value* result = talloc(sizeof(Value));
  if (length(args) == 3) { // checks appropriate length of if statement

    int bool_val = eval(car(args), frame)->i; // checks the int val on the evaluation of the boolean expression

    if (bool_val != 0 && bool_val != 1) { // technically doesn't require strict boolean
      printf("if arg not a boolean.\n"); // 0 and 1 equivalent to #f and #t
      evaluationError();
    }

    if (bool_val) {
      result = eval(car(cdr(args)), frame); // eval 2nd arg
    } else {
      result = eval(car(cdr(cdr(args))), frame); // eval 3rd arg
    }

  } else {
    printf("more/less than 3 args for if\n");
    evaluationError();
  }
  return(result);
}


void evaluationError()
{
  printf("Evaluation ERROR\n");
  texit(EXIT_FAILURE);
  return;
}
