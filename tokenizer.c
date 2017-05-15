#include "value.h"
#include "linkedlist.h"
#include "tokenizer.h"
#include "talloc.h"
#include <stdio.h>
#include <stdlib.h>

// Read all of the input from stdin, and return a linked list consisting of the
// tokens.

/* Throws errors for: (assuming these characters aren't within a string in which case anything goes)

  # being used in a context other than boolean

  +/- being used in a context other than as a single symbol or sign on a number
  (doesn't current allow something like '+)' to be interpreted as single symbol)

  ; being used in a context other than a comment

  digits or '.' as initials being followed by non-digit chars

  not being able to find an EndQuote

  the linked list being improperly constructed with ptrs or cons-cells in the wrong spots
*/

// Max buffer size
#define  MAX_LEN  100

// Helper function prototypes
Value* Open(Value* list);
Value* Close(Value* list);
Value* leadingQuote(Value* list);
Value* leadingDecimal(Value* list, char sign);
Value* leadingDigit(Value* list, char charRead, char sign);
Value* leadingSymbol(Value* list, char charRead, char sign);

Value *tokenize()
{
  char charRead;
  Value *list = makeNull();
  charRead = fgetc(stdin);

  while (charRead != EOF) {

    if (charRead == '"') { // STR //STR definition should have priority over all types

      list = leadingQuote(list);

    } else if (charRead == '(') { // OPEN

      list = Open(list);

    } else if (charRead == ')') { // CLOSE

      list = Close(list);

    } else if (charRead == '#') { // BOOLEAN

      charRead = fgetc(stdin);
      if (charRead == 'f'){ // Flags the boolean with its relevant value

        Value *temp = talloc(sizeof(Value));
        temp->type = BOOL_TYPE;
        temp->i = 0;
        list = cons(temp,list);

      } else if (charRead == 't'){

        Value *temp = talloc(sizeof(Value));
        temp->type = BOOL_TYPE;
        temp->i = 1;
        list = cons(temp,list);

      } else {
        // Error
        fprintf(stderr, "Error: # followed by char other than 'f' or 't'.\n");
        texit(EXIT_FAILURE);
        break;
      }

    } else if (charRead == '+' || charRead == '-'){

      char sign = charRead;
      charRead = fgetc(stdin); // checks next char
      if (charRead == '.') { // Checks what we should project the role of '+/-' to be

        list = leadingDecimal(list, sign);

      } else if (48 <= (int)charRead && (int)charRead <= 57) {

        list = leadingDigit(list, charRead, sign);

      } else if ((int)charRead == 32) { // interpret as single symbol

        list = leadingSymbol(list, charRead, sign);

      } else {
        // Error
        fprintf(stderr, "Error: +/- followed by non-legal.\n");
        texit(EXIT_FAILURE);
      }

    } else if (charRead == '.') { // Leading decimal

      list = leadingDecimal(list, '0');

    } else if (48 <= (int)charRead && (int)charRead <= 57) { // Leading digit

      list = leadingDigit(list, charRead, '0');

    } else if (charRead == ';') { // COMMENT

      charRead = fgetc(stdin);
      if (charRead == ';'){ // signals we have a comment
        while (charRead != EOF && charRead != '\n') {
          // ignores chars until we reach a new line
          charRead = fgetc(stdin);
        }
      } else {
        // error.
        fprintf(stderr, "Error: ; followed by char other than ';'.\n");
        texit(EXIT_FAILURE);
        break;
      }

    } else if ((int)charRead >= 33 && (int)charRead <= 126) {

      list = leadingSymbol(list, charRead, '0'); // The '0' is taking the place of the sign

    }
    charRead = fgetc(stdin);
  } // End of While Loop

   Value *revList = reverse(list);
   return revList;
}

// Helper function creates an open-type value cell
Value* Open(Value* list)
{
  Value *temp = talloc(sizeof(Value));
  temp->type = OPEN_TYPE;
  char *open = talloc(sizeof(char));
  *open = '(';
  temp->s = open;
  list = cons(temp,list);
  return(list);
}

// Helper function creates a close-type value cell
Value* Close(Value* list)
{
  Value *temp = talloc(sizeof(Value));
  temp->type = CLOSE_TYPE;
  char *close = talloc(sizeof(char));
  *close = ')';
  temp->s = close;
  list = cons(temp,list);
  return(list);
}

// Helper function creates a float-type value cell
Value* leadingDecimal(Value* list, char sign)
{
  int flag = 0; // Used in distinguishing 0.1 vs. 0.1( without use of whitespace
  char charRead;
  charRead = fgetc(stdin);
  if (48 <= (int)charRead && (int)charRead <= 57) {  // If we see a number : Leading decimal FLOAT

    Value *temp = talloc(sizeof(Value));
    temp->type = DOUBLE_TYPE;
    int i = 0;
    char buffer[MAX_LEN + 1] = ""; // not an issue since we'll be using atoi and atof
    buffer[0] = '.';

    if (sign == '+') { // handles there being an appended sign
      buffer[0] = '+';
      i = 2;
    } else if (sign == '-') {
      buffer[0] = '-';
      i = 2;
    } else {
      i = 1;
    }

    buffer[i] = charRead;
    while (charRead != ' ') { // While not whitespace or newline

      if (48 <= (int)charRead && (int)charRead <= 57) {

        buffer[i] = charRead; // place digit in buffer
        charRead = fgetc(stdin);

      } else if ((int)charRead == 40 || (int)charRead == 41) { // followed by paren

        if((int)charRead == 40){
          flag = 1; // flag followed by open
        } else {
          flag = 2; // flag followed by close
        }
        break;

      } else {
        // Error
        fprintf(stderr, "Error: Digits followed by chars.\n");
        texit(EXIT_FAILURE);
        break;
      }
      i++;
    }
    temp->d = atof(buffer); // Store the FLOAT from the buffer
    list = cons(temp,list);

    // Comes after to append the token in the right spot of the linked list
    if (flag == 1) {
      list = Open(list);
    } else if (flag == 2) {
      list = Close(list);
    } else {
      // Do nothing
    }

  } else {
    // Error
    fprintf(stderr, "Error: Leading decimal followed by chars.\n");
    texit(EXIT_FAILURE);
  }
  return(list);
}

Value* leadingDigit(Value* list, char charRead, char sign)
{
  int flag = 0;
  Value *temp = talloc(sizeof(Value));
  temp->type = NULL_TYPE;
  char buffer[MAX_LEN + 1] = "";
  int i = 0;

  if (sign == '+') {
    buffer[0] = '+';
    i = 1;
  } else if (sign == '-') {
    buffer[0] = '-';
    i = 1;
  } else {
    i = 0;
  }

  while (charRead != ' ' && charRead != '\n') { // whitespace or newline
    if (charRead == '.') {

      temp->type = DOUBLE_TYPE;
      buffer[i] = '.';
      charRead = fgetc(stdin);

    } else if (48 <= (int)charRead && (int)charRead <= 57) {

      buffer[i] = charRead;
      charRead = fgetc(stdin);

    } else if ((int)charRead == 40 || (int)charRead == 41) { // followed by paren

      if((int)charRead == 40){
        flag = 1;
      } else {
        flag = 2;
      }
      break;

    } else {
      // Error
      fprintf(stderr, "Error: Digits followed by chars.\n");
      texit(EXIT_FAILURE);
      break;
    }
    i++;
  }

  if (temp->type != DOUBLE_TYPE) { // If temp not already declared a double
    temp->type = INT_TYPE;
    temp->i = atoi(buffer); // INT
  } else {
    temp->d = atof(buffer); // FLOAT
  }

  list = cons(temp,list);

  if (flag == 1) {
    list = Open(list);
  } else if (flag == 2) {
    list = Close(list);
  } else {
    // regular case
  }

  return(list);
}

Value* leadingSymbol(Value* list, char charRead, char sign)
{
  int flag = 0;
  Value *temp = talloc(sizeof(Value));
  char *buffer = talloc(sizeof(char)*(MAX_LEN + 1));
  int i;
  if (sign != '+' && sign !='-') { // leading Non +/-
    buffer[0] = charRead;
    i = 1;
    charRead = fgetc(stdin);
    while (((int)charRead >= 33 && (int)charRead <= 126) &&
    ((int)charRead != 40 && (int)charRead != 41)) {
      buffer[i] = charRead;
      i++;
      charRead = fgetc(stdin);
    }

    if ((int)charRead == 40){
      flag = 1;
    } else if ((int)charRead == 41){
      flag = 2;
    } else {
      // regular case
    }

  } else { // +/- on its own
    buffer[0] = sign;
    i = 1;
  }
  buffer[i] = '\0';
  temp->type = SYMBOL_TYPE;
  temp->s = buffer;
  list = cons(temp,list);

  if (flag == 1) {
    list = Open(list);
  } else if (flag == 2) {
    list = Close(list);
  } else {
    // regular case
  }
  return(list);
}

Value* leadingQuote(Value* list)
{
  Value *temp = talloc(sizeof(Value));
  char charRead;
  char *buffer = talloc(sizeof(char)*(MAX_LEN + 1));
  buffer[0] = '"';
  int i = 1;
  charRead = fgetc(stdin);
  while ((int)charRead != 34) { // Read in until endquote is found
    buffer[i] = charRead;
    i++;
    charRead = fgetc(stdin);
    if (charRead == EOF) {
      // Error
      fprintf(stderr, "Error: EndQuote not found\n");
      texit(EXIT_FAILURE);
    }
  }
  buffer[i] = '"';
  buffer[i+1] = '\0';
  temp->type = STR_TYPE;
  temp->s = buffer;
  list = cons(temp,list);
  return(list);
}

// Displays the contents of the linked list as tokens, with type information
void displayTokens(Value *list)
{
  for (;list->type == CONS_TYPE; list = list->c.cdr) { //increment through the list
    switch (list->c.car->type) {
    case INT_TYPE:
        printf("%i : integer\n", list->c.car->i);
        break;
    case DOUBLE_TYPE:
        printf("%f : float\n", list->c.car->d);
        break;
    case STR_TYPE:
        printf("%s", list->c.car->s);
        printf(" : string\n");
        break;
    case NULL_TYPE:
        printf("END\n");
        break;
    case BOOL_TYPE:
        if(list->c.car->i == 0){
          printf("#f : boolean\n");
        } else {
          printf("#t : boolean\n");
        }
        break;
    case SYMBOL_TYPE:
        printf("%s", list->c.car->s);
        printf(" : symbol\n");
        break;
    case OPEN_TYPE:
        printf("( : open\n");
        break;
    case CLOSE_TYPE:
        printf(") : close\n");
        break;
    case PTR_TYPE:
        // Error
        fprintf(stderr, "Improperly constructed list: found ptr_type\n");
        texit(EXIT_FAILURE);
        break;
    case CONS_TYPE:
        // Error
        fprintf(stderr, "Improperly constructed list: found cons_type\n");
        texit(EXIT_FAILURE);
    }
  }
}
