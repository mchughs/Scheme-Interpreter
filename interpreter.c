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
Value* evalIf      (Value* args, Frame* frame);
Value* evalLet     (Value* args, Frame* frame);
void   evalDefine  (Value* args, Frame* frame);
Value* evalLambda  (Value* args, Frame* frame);
Value* evalEach    (Value* args, Frame* frame);
Value* apply       (Value* function, Value* args);

void evaluationError();

Frame* topFrame;

void interpret(Value *tree)
{ // sets up global frame
  topFrame = talloc(sizeof(Frame));
  topFrame->parent = NULL;
  topFrame->bindings = makeNull();

  printTree(tree); // Prints parse tree for comparison
  printf("--> \n");

  Value* evaluated_tree = talloc(sizeof(Value));
  // Increments through and evaluates every S-exp
  while (tree->type != NULL_TYPE) {
    evaluated_tree = eval(car(tree), topFrame);
    printTree(evaluated_tree);
    // to print the proper spacing
    if (evaluated_tree->type == INT_TYPE ||
        evaluated_tree->type == BOOL_TYPE ||
        evaluated_tree->type == STR_TYPE ||
        evaluated_tree->type == DOUBLE_TYPE ||
        evaluated_tree->type == SYMBOL_TYPE
    ) {
      printf("\n");
    }
    tree = cdr(tree);
  }

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
     case VOID_TYPE: {
        printf("???\n");
        break;
     }
     case CLOSURE_TYPE: {
       printf("!!!\n");
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

        if (!strcmp(first->s,"if")) {
            Value* result = evalIf(args,frame); // call helper
            return(result);
        }

        else if (!strcmp(first->s,"let")) {
            Value* result = evalLet(args,frame); // call helper
            return(result);
        }

        else if (!strcmp(first->s,"quote")) {
            return(args);
        }

        else if (!strcmp(first->s,"define")) {
            evalDefine(args, frame); // appends to the topFrame
            args->type = VOID_TYPE; // to prevent printing
            return(args);
        }

        else if (!strcmp(first->s,"lambda")) {
            Value* result = evalLambda(args,frame); // creates a closure
            return(result);
        }

        else {
           // If not a special form, evaluate the first, evaluate the args, then
           // apply the first to the args.
           Value *evaledOperator = eval(first, frame); // The closure
           Value *evaledArgs = evalEach(args, frame);
           return apply(evaledOperator,evaledArgs);
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

  while (true) { // increment through the frames

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

    if (frame->parent != NULL) { // Conditional to break from the loop
      frame = frame->parent;
    } else {
      break;
    }

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

  new_frame->bindings = bindingsList;

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

void evalDefine(Value* args, Frame* frame)
{
  if (length(args) != 2) {
    printf("Too few/many args for define\n");
    evaluationError();
  } else {
    Value* var = talloc(sizeof(Value));
    Value* val = talloc(sizeof(Value));
    Value* var_val = talloc(sizeof(Value));

    var = car(args); // name of the thing being defined
    val = eval(car(cdr(args)), frame); // value associated to the name
    var_val = cons(var,val);
    topFrame->bindings = cons(var_val, topFrame->bindings);
  }
  return;
}

Value* evalLambda(Value* args, Frame* frame)
{
  Value* closure = talloc(sizeof(Value));
  closure->type = CLOSURE_TYPE;

  if (length(args) != 2) {
    printf("Too few/many args for lambda\n");
    evaluationError();
  } else {
    Value* params = talloc(sizeof(Value));
    Value* body = talloc(sizeof(Value));

    params = car(args);
    body = car(cdr(args));

    closure->cl.paramNames = params;
    closure->cl.functionCode = body;
    closure->cl.frame = frame;
  }
  return(closure);
}


Value* evalEach(Value* args, Frame* frame)
{
  Value* tree = makeNull();
  while (args->type != NULL_TYPE) {
    Value* val = talloc(sizeof(Value));
    val = eval(car(args), frame);
    args = cdr(args);
    tree = cons(val, tree);
  }
  return(reverse(tree));
}


Value* apply (Value* function, Value* args)
{
  if (length(args) != length(function->cl.paramNames)) {
    printf("# of requested params != # of passed in args\n");
    evaluationError();
  }

  Frame* frame = talloc(sizeof(Frame));
  frame->parent = function->cl.frame;
  Value* curr = talloc(sizeof(Value));
  curr = function->cl.paramNames;
  frame->bindings = makeNull();

  while ((curr)->type != NULL_TYPE) {
    Value* binding = talloc(sizeof(Value));
    binding = cons(car(curr), car(args)); //args pre-evaluated already
    frame->bindings = cons(binding, frame->bindings);
    curr = cdr(curr);
    args = cdr(args);
  }

  Value* tree = talloc(sizeof(tree));
  tree = eval(function->cl.functionCode, frame);
  return(tree);
}


void evaluationError()
{
  printf("Evaluation ERROR\n");
  texit(EXIT_FAILURE);
  return;
}
