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
Value* evalLetStar (Value* args, Frame* frame);
Value* evalEach    (Value* args, Frame* frame);
Value* apply       (Value* function, Value* args);
void bind(char *name, Value *(*function)(struct Value *), Frame *frame);
Value *primitiveAdd    (Value *args);
Value *primitiveNull   (Value *args);
Value *primitiveCar    (Value *args);
Value *primitiveCdr    (Value *args);
Value *primitiveCons   (Value *args);
Value *primitiveTimes  (Value *args);
Value *primitiveMinus  (Value *args);
Value *primitiveDivide (Value *args);
Value *primitiveModulo (Value *args);
Value *primitiveGreater(Value *args);
Value *primitiveLess   (Value *args);
Value *primitiveEqual  (Value *args);

void evaluationError();

Frame* topFrame;

void interpret(Value *tree)
{ // sets up global frame
  topFrame = talloc(sizeof(Frame));
  topFrame->parent = NULL;
  topFrame->bindings = makeNull();

  bind("+"    ,primitiveAdd ,topFrame);
  bind("null?",primitiveNull,topFrame);
  bind("car"  ,primitiveCar ,topFrame);
  bind("cdr"  ,primitiveCdr ,topFrame);
  bind("cons" ,primitiveCons,topFrame);
  bind("*"    ,primitiveTimes,topFrame);
  bind("-"    ,primitiveMinus,topFrame);
  bind("/"    ,primitiveDivide,topFrame);
  bind("modulo",primitiveModulo,topFrame);
  bind(">"    ,primitiveGreater,topFrame);
  bind("="    ,primitiveEqual,topFrame);
  bind("<"    ,primitiveLess,topFrame);


  printInput(tree); // Prints parse tree for comparison //flag
  printf("--> \n");


  Value* evaluated_tree = talloc(sizeof(Value));
  // Increments through and evaluates every S-exp
  while (tree->type != NULL_TYPE) {
    evaluated_tree = eval(car(tree), topFrame);
    printTree(evaluated_tree);
    // to print the proper spacing
    printf("\n");
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
     case PRIMITIVE_TYPE: {
       printf("NOT sure yet\n");
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
            if (length(args) == 1) {
              return(args);
            } else {
              printf("quote given too many/few arguments\n");
              evaluationError();
            }
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

        else if (!strcmp(first->s,"let*")) {
            Value* result = evalLetStar(args,frame); // call helper
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


Value* evalLetStar (Value* args, Frame* frame)
{
  /*
  Value* result = talloc(sizeof(Value));
  while (args->type != NULL_TYPE) {
    Frame* new_frame = talloc(sizeof(Frame));
    new_frame->parent = frame;
    args = cons(car(car(args)), cdr(args));
    args = evalLet(car(car(args)), new_frame);
    frame = new_frame;
    args = cdr(args);
  }
  */
  return(args);
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
  if (function->type == CLOSURE_TYPE) {

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

  } else {
    return((function->pf)(args));
  }
}


void bind(char *name, Value *(*function)(struct Value *), Frame *frame) {
    // Add primitive functions to top-level bindings list
    Value *value = talloc(sizeof(Value));
    Value* var_val = talloc(sizeof(Value));
    Value* wrapper = talloc(sizeof(Value));

    value->type = PRIMITIVE_TYPE;
    value->pf = function;

    wrapper->type = SYMBOL_TYPE;
    wrapper->s = name;

    var_val = cons(wrapper, value);

    frame->bindings = cons(var_val, frame->bindings);
}


Value *primitiveAdd(Value *args)
{
   // check that args has length 2 and car(args), car(cdr(args)) args are numerical
   if (length(args) == 2) {
     Value* result = talloc(sizeof(Value));
     int int1;
     int int2;
     int intsum;
     double d1;
     double d2;
     double dsum;

     if (car(args)->type == INT_TYPE && car(cdr(args))->type == INT_TYPE) {
       int1 = car(args)->i;
       int2 = car(cdr(args))->i;
       intsum = int1 + int2;
       result->type = INT_TYPE;
       result->i =  intsum;
       return (result);
     } else if (car(args)->type == INT_TYPE && car(cdr(args))->type == DOUBLE_TYPE) {
         int1 = car(args)->i;
         d2 = car(cdr(args))->d;
         dsum = int1 + d2;
         result->type = DOUBLE_TYPE;
         result->d = dsum;
         return (result);
     } else if (car(args)->type == DOUBLE_TYPE && car(cdr(args))->type == INT_TYPE) {
         d1 = car(args)->d;
         int2 = car(cdr(args))->i;
         dsum = d1 + int2;
         result->type = DOUBLE_TYPE;
         result->d = dsum;
         return (result);
     } else if (car(args)->type == DOUBLE_TYPE && car(cdr(args))->type == DOUBLE_TYPE) {
         d1 = car(args)->d;
         d2 = car(cdr(args))->d;
         dsum = d1 + d2;
         result->type = DOUBLE_TYPE;
         result->d = dsum;
         return(result);
     } else {
       printf("+ function not given numbers\n");
       evaluationError();
     }
   } else {
     printf("+ function not given 2 arguments to add\n");
     evaluationError();
   }
   return(args); //error if this step is reached
}

Value *primitiveNull(Value *args)
{
  if (args->type != CONS_TYPE) {
    printf("null? not passed a list\n");
    evaluationError();
  } else {
    Value* result = talloc(sizeof(Value));
    result->type = BOOL_TYPE;
    while (args->type == CONS_TYPE){
      if ((car(args)->type == NULL_TYPE && length(args) == 1) || (car(car(args))->type == NULL_TYPE && length(args) == 1)) { //null? should return true
        result->i = 1;
        return(result);
      }
      args = cdr(args);
    }
    result->i = 0;
    return(result);
  }
  return(args); //error if this step is reached
}


Value *primitiveCar(Value *args)
{
  if (args->type == CONS_TYPE) {
    if (car(args)->type == CONS_TYPE) {
      if (car(car(args))->type == CONS_TYPE && car(car(car(args)))->type != CONS_TYPE) {
          return(car(car(car(args))));
      } else if (car(car(args))->type == CONS_TYPE && car(car(car(args)))->type == CONS_TYPE) {
        Value *wrapper = talloc(sizeof(Value));
        wrapper = makeNull();
        wrapper = cons(car(car(car(args))), wrapper);
        return(wrapper);
      }
    }
  }
  printf("car args not a list/ is a null list\n");
  evaluationError();
  return(args); //error if this step is reached
}


Value *primitiveCdr(Value *args)
{
  if (args->type == CONS_TYPE) {
    if (car(args)->type == CONS_TYPE && car(car(args))->type != NULL_TYPE) {
      if (car(car(args))->type == CONS_TYPE && cdr(car(car(args)))->type != CONS_TYPE) {
        return(cdr(car(car(args))));
      } else if (car(car(args))->type == CONS_TYPE && cdr(car(car(args)))->type == CONS_TYPE) {
        Value *wrapper = talloc(sizeof(Value));
        wrapper = makeNull();
        wrapper = cons(cdr(car(car(args))), wrapper);
        return(wrapper);
      } else {
        Value* null = talloc(sizeof(Value));
        null->type = NULL_TYPE;
        return(null);
      }
    }
  }
  printf("cdr args not a list/ is a null list\n");
  evaluationError();
  return(args); //error if this step is reached
}


Value *primitiveCons(Value *args)
{
  if (length(args) == 2) {
    Value* consCell = talloc(sizeof(Value));
    Value* wrapper  = talloc(sizeof(Value));
    Value* a = car(args);
    Value* b = car(cdr(args));

    //remove useless layers
    while (a->type == CONS_TYPE && cdr(a)->type == NULL_TYPE) {
      a = car(a);
    }
    while (b->type == CONS_TYPE && cdr(b)->type == NULL_TYPE) {
      b = car(b);
    }

    wrapper = makeNull();
    consCell = makeNull();
    if (b->type != NULL_TYPE) {
      consCell = cons(b, consCell);
    }
    if (a->type != NULL_TYPE) {
      consCell = cons(a, consCell);
    }
    consCell = cons(consCell, wrapper);

    return(consCell);
  } else {
    printf("Given more/less than 2 arguments for cons\n");
    evaluationError();
  }
  return(args); //error if this step is reached
}


Value *primitiveTimes(Value *args)
{
  double product = 1;
  Value* result = talloc(sizeof(Value));
  result->type = DOUBLE_TYPE;
  while (args->type != NULL_TYPE) {
    if (car(args)->type == INT_TYPE) {
      product = (double)car(args)->i * product;
    } else if (car(args)->type == DOUBLE_TYPE) {
      product = car(args)->d * product;
    } else {
      printf("* given non number input\n");
      evaluationError();
    }
    args = cdr(args);
  }
  result->d = product;
  return(result);
}

Value *primitiveMinus(Value *args)
{
// check that args has length 2 and car(args), car(cdr(args)) args are numerical
if (length(args) == 2) {
  Value* result = talloc(sizeof(Value));
  int int1;
  int int2;
  int intsum;
  double d1;
  double d2;
  double dsum;

    if (car(args)->type == INT_TYPE && car(cdr(args))->type == INT_TYPE) {
      int1 = car(args)->i;
      int2 = car(cdr(args))->i;
      intsum = int1 - int2;
      result->type = INT_TYPE;
      result->i =  intsum;
      return (result);
    } else if (car(args)->type == INT_TYPE && car(cdr(args))->type == DOUBLE_TYPE) {
        int1 = car(args)->i;
        d2 = car(cdr(args))->d;
        dsum = int1 - d2;
        result->type = DOUBLE_TYPE;
        result->d = dsum;
        return (result);
    } else if (car(args)->type == DOUBLE_TYPE && car(cdr(args))->type == INT_TYPE) {
        d1 = car(args)->d;
        int2 = car(cdr(args))->i;
        dsum = d1 - int2;
        result->type = DOUBLE_TYPE;
        result->d = dsum;
        return (result);
    } else if (car(args)->type == DOUBLE_TYPE && car(cdr(args))->type == DOUBLE_TYPE) {
        d1 = car(args)->d;
        d2 = car(cdr(args))->d;
        dsum = d1 - d2;
        result->type = DOUBLE_TYPE;
        result->d = dsum;
        return(result);
    } else {
      printf("- function not given numbers\n");
      evaluationError();
    }
  } else {
    printf("- function not given 2 arguments to subtract\n");
    evaluationError();
  }
  return(args); //error if this step is reached
}


Value *primitiveDivide(Value *args)
{
  if (length(args) == 2) {
    Value* result = talloc(sizeof(Value));
    if (car(cdr(args))->i == 0) {
      printf("Don't divide by 0\n");
      evaluationError();
    }
    if (car(args)->type == INT_TYPE && car(cdr(args))->type == INT_TYPE) {
      if ((car(args)->i % car(cdr(args))->i) == 0) { //checks for even division
        result->i = car(args)->i / car(cdr(args))->i;
        result->type = INT_TYPE;
      } else {
        result->d = car(args)->i / (double)car(cdr(args))->i;
        result->type = DOUBLE_TYPE;
      }
      return(result);
    } else if (car(args)->type == INT_TYPE && car(cdr(args))->type == DOUBLE_TYPE) {
      result->d = car(args)->i / car(cdr(args))->d;
      result->type = DOUBLE_TYPE;
      return(result);
    } else if (car(args)->type == DOUBLE_TYPE && car(cdr(args))->type == INT_TYPE) {
      result->d = car(args)->d / car(cdr(args))->i;
      result->type = DOUBLE_TYPE;
      return(result);
    } else if (car(args)->type == DOUBLE_TYPE && car(cdr(args))->type == DOUBLE_TYPE) {
      result->d = car(args)->d / car(cdr(args))->d;
      result->type = DOUBLE_TYPE;
      return(result);
    } else {
      printf("divide function not given numbers\n");
      evaluationError();
    }
  } else {
    printf("too many/few args for divide \n");
    evaluationError();
  }
  return(args);
}


Value *primitiveModulo(Value *args)
{
  if (length(args) == 2) {
    if (car(args)->type == INT_TYPE && car(cdr(args))->type == INT_TYPE) {
      Value* result = talloc(sizeof(Value));
      result->type = INT_TYPE;
      result->i = car(args)->i % car(cdr(args))->i;
      if (result->i < 0) {
        result->i = result->i + car(cdr(args))->i;
      }
      return(result);
    } else {
      printf("modulo function not given integers\n");
      evaluationError();
    }
  } else {
    printf("too many/few args for modulo \n");
    evaluationError();
  }
  return(args);
}


Value *primitiveGreater(Value *args)
{
  Value* result = talloc(sizeof(Value));
  result->type = BOOL_TYPE;
  int int1;
  int int2;
  double d1;
  double d2;
  if (length(args) == 2) {
    if (car(args)->type == INT_TYPE && car(cdr(args))->type == INT_TYPE) {
      int1 = car(args)->i;
      int2 = car(cdr(args))->i;
      if (int1 > int2) {
        result->i = 1;
      } else {
        result->i = 0;
      }
      return (result);
    } else if (car(args)->type == INT_TYPE && car(cdr(args))->type == DOUBLE_TYPE) {
        int1 = car(args)->i;
        d2 = car(cdr(args))->d;
        if (int1 > d2) {
          result->i = 1;
        } else {
          result->i = 0;
        }
        return (result);
    } else if (car(args)->type == DOUBLE_TYPE && car(cdr(args))->type == INT_TYPE) {
        d1 = car(args)->d;
        int2 = car(cdr(args))->i;
        if (d1 > int2) {
          result->i = 1;
        } else {
          result->i = 0;
        }
        return (result);
    } else if (car(args)->type == DOUBLE_TYPE && car(cdr(args))->type == DOUBLE_TYPE) {
        d1 = car(args)->d;
        d2 = car(cdr(args))->d;
        if (d1 > d2) {
          result->i = 1;
        } else {
          result->i = 0;
        }
        return(result);
    } else {
      printf("> function not given numbers\n");
      evaluationError();
    }
  } else {
    printf("too many/few args for > \n");
    evaluationError();
  }
  return(args);
}


Value *primitiveLess(Value *args)
{
  if (length(args) == 2) {
    Value* result = talloc(sizeof(Value));
    result->type = BOOL_TYPE;
    int int1;
    int int2;
    double d1;
    double d2;
    if (car(args)->type == INT_TYPE && car(cdr(args))->type == INT_TYPE) {
      int1 = car(args)->i;
      int2 = car(cdr(args))->i;
      if (int1 < int2) {
        result->i = 1;
      } else {
        result->i = 0;
      }
      return (result);
    } else if (car(args)->type == INT_TYPE && car(cdr(args))->type == DOUBLE_TYPE) {
        int1 = car(args)->i;
        d2 = car(cdr(args))->d;
        if (int1 < d2) {
          result->i = 1;
        } else {
          result->i = 0;
        }
        return (result);
    } else if (car(args)->type == DOUBLE_TYPE && car(cdr(args))->type == INT_TYPE) {
        d1 = car(args)->d;
        int2 = car(cdr(args))->i;
        if (d1 < int2) {
          result->i = 1;
        } else {
          result->i = 0;
        }
        return (result);
    } else if (car(args)->type == DOUBLE_TYPE && car(cdr(args))->type == DOUBLE_TYPE) {
        d1 = car(args)->d;
        d2 = car(cdr(args))->d;
        if (d1 < d2) {
          result->i = 1;
        } else {
          result->i = 0;
        }
        return(result);
    } else {
      printf("< function not given numbers\n");
      evaluationError();
    }
  } else {
    printf("too many/few args for < \n");
    evaluationError();
  }
  return(args);
}


Value *primitiveEqual(Value *args)
{
  if (length(args) == 2) {
    Value* result = talloc(sizeof(Value));
    result->type = BOOL_TYPE;
    int int1;
    int int2;
    double d1;
    double d2;
    if (car(args)->type == INT_TYPE && car(cdr(args))->type == INT_TYPE) {
      int1 = car(args)->i;
      int2 = car(cdr(args))->i;
      if (int1 == int2) {
        result->i = 1;
      } else {
        result->i = 0;
      }
      return (result);
    } else if (car(args)->type == INT_TYPE && car(cdr(args))->type == DOUBLE_TYPE) {
        int1 = car(args)->i;
        d2 = car(cdr(args))->d;
        if (int1 == d2) {
          result->i = 1;
        } else {
          result->i = 0;
        }
        return (result);
    } else if (car(args)->type == DOUBLE_TYPE && car(cdr(args))->type == INT_TYPE) {
        d1 = car(args)->d;
        int2 = car(cdr(args))->i;
        if (d1 == int2) {
          result->i = 1;
        } else {
          result->i = 0;
        }
        return (result);
    } else if (car(args)->type == DOUBLE_TYPE && car(cdr(args))->type == DOUBLE_TYPE) {
        d1 = car(args)->d;
        d2 = car(cdr(args))->d;
        if (d1 == d2) {
          result->i = 1;
        } else {
          result->i = 0;
        }
        return(result);
    } else {
      printf("= function not given numbers\n");
      evaluationError();
    }
  } else {
    printf("too many/few args for = \n");
    evaluationError();
  }
  return(args);
}


void evaluationError()
{
  printf("Evaluation ERROR\n");
  texit(EXIT_FAILURE);
  return;
}
