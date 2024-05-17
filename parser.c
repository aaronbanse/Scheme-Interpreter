#include "item.h"
#include <stdio.h>
#include "linkedlist.h"
#include "talloc.h"

//prints syntax error message then exits, freeing all memory used
void parser_error(char *message) {
  printf("Syntax error: %s\n", message);
  texit(1);
}

//prints the value of token tree, depending on the type
void printToken(Item *tree) {
  switch (tree->type) {
  case INT_TYPE:
    printf("%i",tree->i);
    break;
  case BOOL_TYPE:
    printf("#%c",tree->i?'t':'f');
    break;
  case DOUBLE_TYPE:
    printf("%lf",tree->d);
    break;
  case STR_TYPE:
    printf("\"%s\"",tree->s);
    break;
  case NULL_TYPE:
    printf("()");
    break;
  default:
    printf("%s",tree->s);
    break;
  }
}

// Takes a list of tokens from a Scheme program, and returns a pointer to a
// parse tree representing that program.
Item *parse(Item *tokens) {
  Item *stack=makeNull();
  int nestingLevel=0;
  while(tokens->type==CONS_TYPE) {
    Item *token=car(tokens);
    if (token->type == OPEN_TYPE || token->type == OPENBRACKET_TYPE)
      nestingLevel++;
    if(token->type==CLOSE_TYPE) {
      if(nestingLevel<=0) parser_error("too many close parentheses");
      token=makeNull();
      while(car(stack)->type!=OPEN_TYPE) {
        token=cons(car(stack),token);//make token into a reversed list of
                                       //things on the stack
        if(cdr(stack)->type!=CONS_TYPE) parser_error("too many close parentheses");
        stack=cdr(stack);//pop the open off the stack

        }
        nestingLevel--;
      stack=cdr(stack);
    } else if(token->type==CLOSEBRACKET_TYPE) {
      if(nestingLevel<=0) parser_error("too many close parentheses");
      token=makeNull();
      while(car(stack)->type!=OPENBRACKET_TYPE) {
        token=cons(car(stack),token);//make token into a reversed list of
                                       //things on the stack
        if(cdr(stack)->type!=CONS_TYPE) parser_error("too many close parentheses");
        stack=cdr(stack);//pop the openbracket off the stack
      }
      nestingLevel--;
      stack=cdr(stack);
    }
    stack=cons(token,stack);//add the token to the stack
    tokens=cdr(tokens);//pop
  }
  if(nestingLevel!=0) parser_error("not enough close parentheses");
  return reverse(stack);
  //return (stack);
}


// Prints the tree to the screen in a readable fashion. It should look just like
// Scheme code; use parentheses to indicate subtrees.
void printTree(Item *tree) {
  while(tree->type==CONS_TYPE){
    if(car(tree)->type==CONS_TYPE) {
      printf("(");
      printTree(car(tree));
      printf(")");
    } else {
      printToken(car(tree));
    }
    if(cdr(tree)->type!=NULL_TYPE)
      printf(" ");
    tree=cdr(tree);
  }
}
