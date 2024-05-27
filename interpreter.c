#include <string.h>
#include <stdio.h>
#include "item.h"
#include "linkedlist.h"
#include "talloc.h"
#include "parser.h"
#include "interpreter.h"

// throws an error and exits
void evaluationerror(char* mes) {
  printf("Evaluation error: %s\n", mes);
  texit(1);
}

// apply a function and return the value
Item *apply(Item *function, Item *args) {
  if(function->type != CLOSURE_TYPE) evaluationerror("not a function");
  Frame *appFrame = talloc(sizeof(Frame));
  appFrame->parent = function->cl.frame;
  appFrame->bindings = makeNull();

  if (function->cl.paramNames->type == SYMBOL_TYPE) {
    // variable length args

    Item *val = cons(args, makeNull());
    Item *binding = cons(function->cl.paramNames, val);
    appFrame->bindings = cons(binding, appFrame->bindings);
  } 
  else if (function->cl.paramNames->type == CONS_TYPE) {
    // set list of args (or no args)
    Item *parNames = function->cl.paramNames;
    while(args->type == CONS_TYPE && parNames->type == CONS_TYPE) {
      // construct binding
      Item *val = cons(car(args), makeNull());
      Item *binding = cons(car(parNames), val);
      appFrame->bindings = cons(binding, appFrame->bindings);
      args = cdr(args);
      parNames = cdr(parNames);
    }
    if(args->type == CONS_TYPE) evaluationerror("too many arguments given");
    if(parNames->type == CONS_TYPE) evaluationerror("not enough arguments given");
  } 
  else if (function->cl.paramNames->type != NULL_TYPE) {
    evaluationerror("invalid parameter names");
  }
  
  return eval(function->cl.functionCode, appFrame);
}

// takes in a list of S-expressions in the form of abstract syntax trees,
// calls eval on each, and prints the result.
void interpret(Item *tree) {
  Frame *frame = talloc(sizeof(Frame));
  frame->bindings=makeNull();
  frame->parent=NULL;
  while(!isNull(tree)) {
    //print tree
    Item *evaluated = eval(car(tree), frame);
    printTree(cons(evaluated, makeNull()));
    if(evaluated->type != VOID_TYPE) printf(" ");
    
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
    case CLOSURE_TYPE:
      {
      return tree;
      break;
      }
    case SYMBOL_TYPE: {
      //Code to implement what should happen if we have a symbol
      //check through frames for binding of variable
      Frame *search_frame=frame;
      Item *binding;
      while(search_frame!=NULL) {
        binding=search_frame->bindings;
        while (!isNull(binding)) {
          if (!strcmp(car(car(binding))->s, tree->s)) {
            // this symbol has a binding
            return car(cdr(car(binding)));//return the value of the binding
          }
          //check next binding
          binding = cdr(binding);
        }
        //check next frame
        search_frame=search_frame->parent;
      }
      //we broke because we ran out of frames to check, so our symbol is unbound
      evaluationerror("unbound variable");
      return makeNull();
      break;
    }  
    case CONS_TYPE: {
        Item *first = car(tree);
        Item *args = cdr(tree);

        // Some error checking here (omitted from sample code) before moving to cases
        //if(first->type!=SYMBOL_TYPE) evaluationerror("cannot evaluate");

        // if first is list we need to evaluate it
        if(first->type == CONS_TYPE) {
          first = eval(first, frame);
        }

        // apply closure
        if (first->type == CLOSURE_TYPE) {
          return apply(first, args);
        }

        if(first->type != SYMBOL_TYPE) evaluationerror("cannot evaluate non-function");
        
        
        // if statement
        if (!strcmp(first->s,"if")) {
          if(length(args)<3) evaluationerror("");
          Item *result=eval(car(args),frame);
          if(result->type!=BOOL_TYPE) evaluationerror("");
          if(result->i) return eval(car(cdr(args)),frame);
          else return eval(car(cdr(cdr(args))),frame);
        }

        // let statement
        if (!strcmp(first->s, "let")) {
          Frame *newFrame = talloc(sizeof(Frame));
          newFrame->bindings = car(args);

          newFrame->parent = frame;
          if(length(args)<2) evaluationerror("");

          // verify formatting of let bindings
          Item *bindingArg = car(args);
          Item *symbolList = makeNull();
          while(bindingArg->type == CONS_TYPE) {
            Item *binding = car(bindingArg);
            if(binding->type != CONS_TYPE) evaluationerror("");
            if(length(binding) != 2 || car(binding)->type != SYMBOL_TYPE) evaluationerror("");

            // search prev symbols for duplicates
            Item *searchDup = symbolList;
            while(searchDup->type == CONS_TYPE) {
              if(!strcmp(car(symbolList)->s, car(binding)->s)) evaluationerror("");
              searchDup = cdr(searchDup);
            }
            
            symbolList = cons(car(binding), symbolList);

            // evaluate binding
            binding->c.cdr->c.car = eval(car(cdr(binding)), frame);
            bindingArg = cdr(bindingArg);
          }
          if(bindingArg->type != NULL_TYPE) evaluationerror("");

          
          // go to last S-expression in let
          while(cdr(args)->type != NULL_TYPE) args = cdr(args);
          
          return eval(car(args), newFrame);
        }

        // define statement
        if (!strcmp(first->s, "define")) {
          if(length(args) < 2) evaluationerror("");
          Item *var = car(args);
          Item *binding = eval(car(cdr(args)), frame);
          if(var->type != SYMBOL_TYPE) evaluationerror("");

          Item *b = cons(binding, makeNull());
          Item *v = cons(var, b);
          frame->bindings = cons(v, frame->bindings);
          
          Item *voidReturn = talloc(sizeof(Item));
          voidReturn->type = VOID_TYPE;
          return voidReturn;
        }

        // lambda statement
        if (!strcmp(first->s, "lambda")) {
          if(length(args) < 2) evaluationerror("not enough arguments");
          Item *closure = talloc(sizeof(Item));
          closure->type = CLOSURE_TYPE;
          closure->cl.frame = frame;
          closure->cl.paramNames = car(args);

          if(closure->cl.paramNames->type == CONS_TYPE) {
            // check for valid param formatting
            Item *checkParam = closure->cl.paramNames;
            while(checkParam->type == CONS_TYPE) {
              if (car(checkParam)->type != SYMBOL_TYPE) evaluationerror("parameter must be symbol");
              
              // search prev symbols for duplicates
              Item *searchDup = cdr(checkParam);
              while(searchDup->type == CONS_TYPE) {
                if(!strcmp(car(checkParam)->s, car(searchDup)->s)) evaluationerror("duplicate parameter");
                searchDup = cdr(searchDup);
              }
              checkParam = cdr(checkParam);
            }
          } else if(closure->cl.paramNames->type != SYMBOL_TYPE && closure->cl.paramNames->type != NULL_TYPE) {
            evaluationerror("parameters must be list");
          }


          closure->cl.functionCode = car(cdr(args));
          return closure;
        }

        // quote statement
        if (!strcmp(first->s, "quote")) {
          if(length(args)!=1) evaluationerror("need one arguments");
          return car(args);
        }

        else {
          // try to apply function
          return apply(eval(first, frame), args);
        }
        break;
      }
      default: {
        evaluationerror("default");
        return makeNull();
        break;
      }
    }

}