#include <string.h>
#include <stdio.h>
#include "item.h"
#include "linkedlist.h"
#include "talloc.h"
#include "parser.h"
#include "interpreter.h"

// throws an error and exits
void evaluationError() {
  printf("Evaluation error\n");
  texit(1);
}

// takes in a list of S-expressions in the form of abstract syntax trees,
// calls eval on each, and prints the result.
void interpret(Item *tree) {
  Frame *frame = talloc(sizeof(Frame));
  frame->bindings=makeNull();
  frame->parent=NULL;
  while(!isNull(tree)) {
    //print tree
    printTree(cons(eval(car(tree), frame), makeNull()));
    printf(" ");
    tree=cdr(tree);
  }
  printf("\n");
}

// evaluates an S-expression in the form of an abstract syntax tree
Item *eval(Item *tree, Frame *frame) {
     switch (tree->type)  {
     case INT_TYPE:
     case DOUBLE_TYPE:
     case STR_TYPE:
     case BOOL_TYPE:
       //case NULL_TYPE:
       {
       return tree;
       break;
       }
     case SYMBOL_TYPE: {
       //Code to implement what should happen if we have a symbol
       //check through frames for binding of variable
       Frame *search_frame=frame;
       Item *binding=frame->bindings;
       while(search_frame!=NULL) {
         binding=search_frame->bindings;
         while (!isNull(binding)) {
           if (!strcmp(car(car(binding))->s, tree->s)) {
             // this symbol has a binding
             return eval(car(cdr(car(binding))), search_frame->parent);//return the value of the binding
           }
           //check next binding
           binding = cdr(binding);
         }
         //check next frame
         search_frame=search_frame->parent;
       }
       //we broke because we ran out of frames to check, so our symbol is unbound
       evaluationError();
       break;
     }  
     case CONS_TYPE: {
        Item *first = car(tree);
        Item *args = cdr(tree);

        // Some error checking here (omitted from sample code) before moving to cases
        if(first->type!=SYMBOL_TYPE) evaluationError();

        //are we at an if statement?
        if (!strcmp(first->s,"if")) {
          if(length(args)<3) evaluationError();
          Item *result=eval(car(args),frame);
          if(result->type!=BOOL_TYPE) evaluationError();
          if(result->i) return eval(car(cdr(args)),frame);
          else return eval(car(cdr(cdr(args))),frame);
        }

        if (!strcmp(first->s, "let")) {
          Frame *newFrame = talloc(sizeof(Frame));
          newFrame->bindings = car(args);
          newFrame->parent = frame;
          if(length(args)<2) evaluationError();

          // verify formatting of let bindings
          Item *bindingArg = car(args);
          Item *symbolList = makeNull();
          while(bindingArg->type == CONS_TYPE) {
            Item *binding = car(bindingArg);
            if(binding->type != CONS_TYPE) evaluationError();
            if(length(binding) != 2 || car(binding)->type != SYMBOL_TYPE) evaluationError();

            // search prev symbols for duplicates
            Item *searchDup = symbolList;
            while(searchDup->type == CONS_TYPE) {
              if(!strcmp(car(symbolList)->s, car(binding)->s)) evaluationError();
              searchDup = cdr(searchDup);
            }
            
            symbolList = cons(car(binding), symbolList);
            bindingArg = cdr(bindingArg);
          }
          if(bindingArg->type != NULL_TYPE) evaluationError();

          
          // go to last S-expression in let
          while(cdr(args)->type != NULL_TYPE) args = cdr(args);
          
          return eval(car(args), newFrame);
        }

        if (!strcmp(first->s, "quote")) {
          if(length(args)!=1) evaluationError();
          return car(args);
        }


        else {
           // not a recognized special form
           evaluationError();
        }
        break;
     }

       //....
     default:
       return tree;
    }    
     //....
     return tree;
     
}
