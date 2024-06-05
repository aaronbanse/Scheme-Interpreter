#include <string.h>
#include <stdio.h>
#include "item.h"
#include "linkedlist.h"
#include "talloc.h"
#include "parser.h"
#include "interpreter.h"
#include <assert.h>

// throws an error and exits
void evaluationError(char* mes) {
  printf("Evaluation error: %s\n", mes);
  texit(1);
}

// primitive functions

Item *primitiveNull(Item *args) {
  if(length(args) != 1) evaluationError("null? takes 1 argument");
  Item *result = makeNull();
  result->type = BOOL_TYPE;
  result->i = isNull(car(args));
  return result;
}

Item *primitiveCar(Item *args) {
  if(length(args) != 1) evaluationError("car takes 1 argument");
  if(car(args)->type != CONS_TYPE) evaluationError("car argument must be cons cell");
  return car(car(args));
}

Item *primitiveCdr(Item *args) {
  if(length(args) != 1)
    evaluationError("cdr takes 1 argument");
  if(car(args)->type != CONS_TYPE)
    evaluationError("cdr argument must be cons cell");
  return cdr(car(args));
}

Item *primitiveCons(Item *args) {
  if(length(args) != 2) evaluationError("cons takes 2 argument");
  return cons(car(args), car(cdr(args)));
}


Item *primitiveAppend(Item *args) {
  if(length(args) != 2) evaluationError("append takes 2 argument");
  Item *newList = makeNull();
  Item *cur = car(args);
  while(cur->type == CONS_TYPE) {
    newList=cons(car(cur),newList);
    cur=cdr(cur);
  }
  if(cur->type != NULL_TYPE)
    evaluationError("append takes a null-terminated list in first argument");
  cur = newList;
  Item *result=car(cdr(args));
  while(cur->type==CONS_TYPE) {
    result=cons(car(cur),result);
    cur=cdr(cur);
  }
  return result;
}

Item *primitivePlus(Item *args) {
  double sum = 0;//every 32bit integer can be represented as a double
  bool is_double = false;
  while(args->type == CONS_TYPE) {
    if(car(args)->type != DOUBLE_TYPE && car(args)->type != INT_TYPE) evaluationError("+ only takes numbers as arguments");
    if(car(args)->type == DOUBLE_TYPE) {
      is_double = true;
      sum += car(args)->d;
    } else {
      sum += (double)car(args)->i;
    }
    args=cdr(args);
  }
  Item *result = makeNull();
  if(is_double) {
    result->type = DOUBLE_TYPE;
    result->d = sum;
  } else {
    result->type = INT_TYPE;
    result->i = (int)sum;
  }
  return result;
}

Item *primitiveMinus(Item *args) {
  if(args->type!=CONS_TYPE) evaluationError("primitive - requires atleast one argument");
  Item *lead=car(args);
  Item *newLead=makeNull();
  if(lead->type==INT_TYPE) {
    newLead->type=INT_TYPE;
    newLead->i=-(lead->i);
  } else if(lead->type==DOUBLE_TYPE) {
    newLead->type=DOUBLE_TYPE;
    newLead->d=-(lead->d);
  }
  Item *result = primitivePlus(cons(newLead,cdr(args)));
  if(result->type==INT_TYPE)
    result->i=-result->i;
  else if(result->type==DOUBLE_TYPE)
    result->d=-result->d;
  return result;
}

Item *primitiveMult(Item *args) {
  double product=1;
  bool sawDouble=false;
  while(args->type!=NULL_TYPE) {
    if(car(args)->type==INT_TYPE)
      product*=(double)car(args)->i;
    if(car(args)->type==DOUBLE_TYPE) {
      sawDouble=true;
      product*=car(args)->d;
    }
    args=cdr(args);
  }
  Item *result=makeNull();
  if(sawDouble){
    result->type=DOUBLE_TYPE;
    result->d=product;
  } else {
    result->type=INT_TYPE;
    result->i=(int)product;
  }
  return result;
}

Item *primitiveDiv(Item *args) {
  if(length(args)!=2)
    evaluationError("/ only supports 2 arguments");
  Item *result=makeNull();
  if(car(args)->type==INT_TYPE
     && car(cdr(args))->type == INT_TYPE
     && car(args)->i % car(cdr(args))->i ==0) {
    result->type=INT_TYPE;
    result->i = div(car(args)->i,car(cdr(args))->i).quot;
  } else {
    double x,y;
    if(car(args)->type==INT_TYPE) {
      x=(double)car(args)->i;
    } else if(car(args)->type==DOUBLE_TYPE) {
      x=(double)car(args)->d;
    } else evaluationError("/ requires two numerical arguments");
    if(car(cdr(args))->type==INT_TYPE) {
      y=(double)car(cdr(args))->i;
    } else if(car(cdr(args))->type==DOUBLE_TYPE) {
      y=(double)car(cdr(args))->d;
    } else evaluationError("/ requires two numerical arguments");
    result->type=DOUBLE_TYPE;
    result->d=x/y;
  }
  return result;
}

//prints the remainder of two integers
Item *primitiveModulo(Item *args) {
  if(length(args)!=2)
    evaluationError("modulo only supports 2 arguments");
  if(car(args)->type!=INT_TYPE || car(cdr(args))->type!=INT_TYPE)
    evaluationError("modulo only supports integer arguments");
  Item *result=makeNull();
  result->type=INT_TYPE;
  result->i=car(args)->i % car(cdr(args))->i;
  return result;
}


Item *primitiveLessThan(Item *args) {
  if(args->type!=CONS_TYPE)
    evaluationError("primitive < requires atleast one argument");
  double current=0.0/0.0;//initialize to NaN. always compares false
  while(args->type==CONS_TYPE) {
    if(car(args)->type !=INT_TYPE
       && car(args)->type !=DOUBLE_TYPE)
      evaluationError("primitive < requires numbers as arguments");
    else if(car(args)->type==INT_TYPE) {
      if(current >= (double)car(args)->i) break;
      current=(double)car(args)->i;
    } else if(car(args)->type==DOUBLE_TYPE) {
      if(current >= car(args)->d) break;
      current=car(args)->d;
    }
    args=cdr(args);
  }
  Item *result=makeNull();
  result->type=BOOL_TYPE;
  result->i=isNull(args);
  return result;
}

Item *primitiveGreaterThan(Item *args) {
  if(args->type!=CONS_TYPE)
    evaluationError("primitive > requires atleast one argument");
  double current=0.0/0.0;//initialize to NaN. always compares false
  while(args->type==CONS_TYPE) {
    if(car(args)->type !=INT_TYPE
       && car(args)->type !=DOUBLE_TYPE)
      evaluationError("primitive > requires numbers as arguments");
    else if(car(args)->type==INT_TYPE) {
      if(current <= (double)car(args)->i) break;
      current=(double)car(args)->i;
    } else if(car(args)->type==DOUBLE_TYPE) {
      if(current <= car(args)->d) break;
      current=car(args)->d;
    }
    args=cdr(args);
  }
  Item *result=makeNull();
  result->type=BOOL_TYPE;
  result->i=isNull(args);
  return result;
}

Item *primitiveEqualTo(Item *args) {
  if(args->type!=CONS_TYPE)
    evaluationError("primitive = requires atleast one argument");
  double current;
  if(car(args)->type == INT_TYPE)
    current=(double)car(args)->i;
  if(car(args)->type == DOUBLE_TYPE)
    current=car(args)->d;
  while(args->type==CONS_TYPE) {
    if(car(args)->type !=INT_TYPE
       && car(args)->type !=DOUBLE_TYPE)
      evaluationError("primitive = requires numbers as arguments");
    else if(car(args)->type==INT_TYPE) {
      if(current != (double)car(args)->i) break;
      current=(double)car(args)->i;
    } else if(car(args)->type==DOUBLE_TYPE) {
      if(current != car(args)->d) break;
      current=car(args)->d;
    }
    args=cdr(args);
  }
  Item *result=makeNull();
  result->type=BOOL_TYPE;
  result->i=isNull(args);
  return result;
}

//binds a single var to evaluated expr in frame 
void bind(Item *var, Item *expr, Frame *frame) {
    if(var->type!=SYMBOL_TYPE)
      evaluationError("tried to bind expr to non-symbol");
    //add binding
    Item *binding = cons(var,cons(expr,makeNull()));
    frame->bindings=cons(binding,frame->bindings);
}

//takes a linked list of strings with var names and evaluated expressions
//and adds bindings for all in frame, checking for duplicates
void bindList(Item *vars, Item *exprs, Frame *frame) 
{
  assert(length(vars)==length(exprs));
  Item *symbolSearch;
  while(vars->type == CONS_TYPE) {
    //check for duplicates
    symbolSearch=cdr(vars);
    while(symbolSearch->type==CONS_TYPE) {
      if(!strcmp(car(symbolSearch)->s,car(vars)->s))
        evaluationError("duplicate bound variable in bind");
      symbolSearch=cdr(symbolSearch);
    }
    bind(car(vars),car(exprs),frame);
    vars=cdr(vars);
    exprs=cdr(exprs);
  }
}

//maps eval over trees
Item *multiEval(Item *trees, Frame *frame) {
  Item *result=makeNull();
  while(trees->type==CONS_TYPE) {
    result=cons(eval(car(trees),frame),result);
    trees=cdr(trees);
  }
  return reverse(result);
}

Item *parseBindings(Item *bindings) {
  Item *vars=makeNull();
  Item *exprs=makeNull();
  while(bindings->type==CONS_TYPE) {
    Item *binding = car(bindings);
    if (binding->type != CONS_TYPE
        || length(binding) != 2)
      evaluationError("incorrect binding format in let");
    vars=cons(car(binding),vars);
    exprs=cons(car(cdr(binding)),exprs);
    bindings=cdr(bindings);
  }
  if (bindings->type != NULL_TYPE)
    evaluationError("binding list is not null-terminated in let");
  return cons(vars,exprs);
}

//evaluates a let form
Item *evalLet(Item *args, Frame *frame) {
  if (length(args) < 2)
    evaluationError("too few arguments for let");
  
  Item *bindings=parseBindings(car(args));
  Item *vars=car(bindings);
  Item *exprs=multiEval(cdr(bindings),frame);

  
  Frame *newFrame=talloc(sizeof(Frame));
  newFrame->parent=frame;
  newFrame->bindings=makeNull();

  bindList(vars,exprs,newFrame);

  //evaluate bodys and return result of last S-expression
  Item *bodys=cdr(args);
  Item *result= multiEval(bodys,newFrame);
  while(cdr(result)->type==CONS_TYPE)
    result=cdr(result);
  return car(result);
}

Item *evalLetStar(Item *args, Frame *frame) {
  if (length(args) < 2)
          evaluationError("too few arguments for let");

  Item *bindings=parseBindings(car(args));
  Item *vars=car(bindings);
  Item *exprs=cdr(bindings);


  // bind each var-expr pair to a new frame of its own, with parents chained
  vars=reverse(vars);
  exprs=reverse(exprs);
  Frame *newFrame;
  while (vars->type == CONS_TYPE) {
          newFrame = talloc(sizeof(Frame));
          newFrame->parent = frame;
          newFrame->bindings = makeNull();
          bind(car(vars), eval(car(exprs), frame), newFrame);
          vars=cdr(vars);
          exprs=cdr(exprs);
          frame=newFrame;
  }

  //evaluate bodys and return result of last S-expression
  Item *bodys=cdr(args);
  Item *result= multiEval(bodys,newFrame);
  while(cdr(result)->type==CONS_TYPE)
    result=cdr(result);
  return car(result);
}

//what the fuck???
//The description on the site makes no sense for letrec ¯\_(ツ)_/¯
Item *evalLetRec(Item *args, Frame *frame) {
  if (length(args) < 2)
          evaluationError("too few arguments for let");
  Item *bindings=parseBindings(car(args));
  Item *vars=car(bindings);
  Item *exprs=cdr(bindings);
  Frame *newFrame=talloc(sizeof(Frame));
  newFrame->parent=frame;
  newFrame->bindings=makeNull();
  exprs=multiEval(exprs,newFrame);
  bindList(vars,exprs,newFrame);

  //evaluate bodys and return result of last S-expression
  Item *bodys=cdr(args);
  Item *result= multiEval(bodys,newFrame);
  while(cdr(result)->type==CONS_TYPE)
    result=cdr(result);
  return car(result);
}

Item *evalAnd(Item *args, Frame *frame) {
  Item *result=makeNull();
  result->type=BOOL_TYPE;
  result->i=true;
  while(args->type==CONS_TYPE) {
    result=eval(car(args),frame);
    if(result->type==BOOL_TYPE && result->i==false)
      break;
    args=cdr(args);
  }
  return result;
}

Item *evalOr(Item *args, Frame *frame) {
  Item *result=makeNull();
  result->type=BOOL_TYPE;
  result->i=false;
  while(args->type==CONS_TYPE) {
    result=eval(car(args),frame);
    if(result->type!=BOOL_TYPE || result->i==true)
      break;
    args=cdr(args);
  }
  return result;
}

Frame *searchFrame(Item *var, Frame *frame) {
  Frame *search_frame = frame;
  Item *binding;
  while (search_frame != NULL) {
    binding = search_frame->bindings;
    while (!isNull(binding)) {
      if (!strcmp(car(car(binding))->s, var->s)) // this symbol has a binding
        return search_frame; // stop searching so search_frame refers to the containing
                    // frame
      binding = cdr(binding); // check next binding
    }
    search_frame = search_frame->parent; // check next frame
  } // we broke because we ran out of frames to check, so our symbol is unbound
  evaluationError("unbound variable in set!-form");
  return frame;//this should never be run
}

Item *evalSetBang(Item *args, Frame *frame) {
  if (length(args) != 2)
    evaluationError("set! takes 2 arguments");

  Item *var=car(args);
  Item *expr = eval(car(cdr(args)), frame);
  Frame *search_frame=searchFrame(var,frame);
  bind(var, expr, search_frame); // change binding in the closest frame
                                 // containing a binding for var

  Item *voidReturn = talloc(sizeof(Item));
  voidReturn->type = VOID_TYPE;
  return voidReturn;
}

// bind a primitive function to a symbol in given frame
void primBind(char *name, Item *(*function)(Item *), Frame *frame) {
  Item *symbol = talloc(sizeof(Item));
  symbol->type = SYMBOL_TYPE;
  symbol->s = name;
  Item *functionItem = talloc(sizeof(Item));
  functionItem->type = PRIMITIVE_TYPE;
  functionItem->pf = function;
  bind(symbol, functionItem, frame);
}

// apply a function and return the value
Item *apply(Item *function, Item *args) {
  if (function->type != CLOSURE_TYPE)
    evaluationError("not a function");
  Frame *appFrame = talloc(sizeof(Frame));
  appFrame->parent = function->cl.frame;
  appFrame->bindings = makeNull();

  if (function->cl.paramNames->type == SYMBOL_TYPE) {
    // variable length args

    Item *val = cons(args, makeNull());
    Item *binding = cons(function->cl.paramNames, val);
    appFrame->bindings = cons(binding, appFrame->bindings);
  } else if (function->cl.paramNames->type == CONS_TYPE) {
    // set list of args (or no args)
    Item *parNames = function->cl.paramNames;
    while (args->type == CONS_TYPE && parNames->type == CONS_TYPE) {
      // construct binding
      Item *val = cons(car(args), makeNull());
      Item *binding = cons(car(parNames), val);
      appFrame->bindings = cons(binding, appFrame->bindings);
      args = cdr(args);
      parNames = cdr(parNames);
    }
    if (args->type == CONS_TYPE)
      evaluationError("too many arguments given");
    if (parNames->type == CONS_TYPE)
      evaluationError("not enough arguments given");
  } else if (function->cl.paramNames->type != NULL_TYPE) {
    evaluationError("invalid parameter names");
  }

  Item *result=multiEval(function->cl.functionCode,appFrame);
  while(cdr(result)->type==CONS_TYPE)
    result=cdr(result);
  return car(result);
}

// takes in a list of S-expressions in the form of abstract syntax trees,
// calls eval on each, and prints the result.
void interpret(Item *tree) {
  Frame *frame = talloc(sizeof(Frame));
  frame->bindings = makeNull();
  frame->parent = NULL;

  // set primitive bindings
  primBind("+", primitivePlus, frame);
  primBind("null?", primitiveNull, frame);
  primBind("car", primitiveCar, frame);
  primBind("cdr", primitiveCdr, frame);
  primBind("cons", primitiveCons, frame);
  primBind("append", primitiveAppend, frame);
  primBind("-", primitiveMinus, frame);
  primBind("<", primitiveLessThan, frame);
  primBind(">", primitiveGreaterThan, frame);
  primBind("=", primitiveEqualTo, frame);
  primBind("*", primitiveMult, frame);
  primBind("/", primitiveDiv, frame);
  primBind("modulo", primitiveModulo, frame);

  Item *result;
  while(tree->type==CONS_TYPE) {
    result=eval(car(tree),frame);
    if(result->type!=VOID_TYPE && result->type!=NULL_TYPE)
      printTree(cons(result,makeNull()));
    tree=cdr(tree);
 }
}

// evaluates an S-expression in the form of an abstract syntax tree
Item *eval(Item *tree, Frame *frame) {
  switch (tree->type) {
  case INT_TYPE:
  case DOUBLE_TYPE:
  case STR_TYPE:
  case BOOL_TYPE:
  case CLOSURE_TYPE:
  case PRIMITIVE_TYPE: {
    return tree;
    break;
  }
  case SYMBOL_TYPE: {
    // Code to implement what should happen if we have a symbol
    // check through frames for binding of variable
    Frame *search_frame = frame;
    while (search_frame != NULL) {
      Item *binding = search_frame->bindings;
      while (!isNull(binding)) {
        if (!strcmp(car(car(binding))->s, tree->s)) // this symbol has a binding
          return car(cdr(car(binding))); // return the value of the binding
        binding = cdr(binding);          // check next binding
      }
      search_frame = search_frame->parent; // check next frame
    } // we broke because we ran out of frames to check, so our symbol is
      // unbound
    evaluationError("unbound variable");
    return makeNull();
    break;
  }
  case CONS_TYPE: {
    Item *first = car(tree);
    Item *args = cdr(tree);

    if (first->type == SYMBOL_TYPE) {
      // if statement
      if (!strcmp(first->s, "if")) {
        if (length(args) < 3)
          evaluationError("too few arguments to if");
        Item *result = eval(car(args), frame);
        if (result->type != BOOL_TYPE)
          evaluationError("conditional in if didn't evaluate to bool");
        if (result->i)
          return eval(car(cdr(args)), frame);
        else
          return eval(car(cdr(cdr(args))), frame);
      }
      if (!strcmp(first->s, "cond")) {
        while(args->type==CONS_TYPE) {
          Item *clause=car(args);
          Item *predicate=car(clause);
          Item *result;
          if(isNull(cdr(args))
              && predicate->type==SYMBOL_TYPE
              && !strcmp(car(clause)->s,"else")){
            if(isNull(cdr(clause)))
              evaluationError("invalid clause in cond");
            result->type=BOOL_TYPE;
            result->i=1;
          } else {
            result=eval(predicate,frame);
          }
          if(result->type!=BOOL_TYPE
             || result->i==1)
            {
              Item *evaledClause=multiEval(cdr(clause),frame);
            while(evaledClause->type!=NULL_TYPE) {
              result=car(evaledClause);
              evaledClause=cdr(evaledClause);
            }
            return result;
          }
          args=cdr(args);
        }
        Item *voidReturn=makeNull();
        voidReturn->type=VOID_TYPE;
        return voidReturn;
      }

      // let statement
      if (!strcmp(first->s, "let")) {
        return evalLet(args, frame);
      }
      if (!strcmp(first->s, "let*")) {
        return evalLetStar(args, frame);
      }
      if (!strcmp(first->s, "letrec")) {
        return evalLetRec(args, frame);
      }

      // check binding exists then its the same as define
      if (!strcmp(first->s, "set!"))
        return evalSetBang(args, frame);
      if (!strcmp(first->s, "set-car!")) {
        if (length(args) != 2)
          evaluationError("set-car! takes 2 arguments");
        Item *pair=eval(car(args),frame);
        Item *obj=car(cdr(args));
        if(pair->type!=CONS_TYPE)
          evaluationError("set-car! requires CONS cell as first input");
        pair->c.car=eval(obj,frame);
        Item *voidReturn = talloc(sizeof(Item));
        voidReturn->type = VOID_TYPE;
        return voidReturn;
      }
      if (!strcmp(first->s, "set-cdr!")) {
        if (length(args) != 2)
          evaluationError("set-cdr! takes 2 arguments");
        Item *pair=eval(car(args),frame);
        Item *obj=car(cdr(args));
        if(pair->type!=CONS_TYPE)
          evaluationError("set-cdr! requires CONS cell as first input");
        pair->c.cdr=eval(obj,frame);
        Item *voidReturn = talloc(sizeof(Item));
        voidReturn->type = VOID_TYPE;
        return voidReturn;
      }

      if (!strcmp(first->s, "and"))
        return evalAnd(args,frame);
      if (!strcmp(first->s, "or"))
        return evalOr(args,frame);

      // define statement
      if (!strcmp(first->s, "define")) {
        if (length(args) < 2)
          evaluationError("too few arguments for define");
        bind(car(args), eval(car(cdr(args)), frame), frame);
        Item *voidReturn = talloc(sizeof(Item));
        voidReturn->type = VOID_TYPE;
        return voidReturn;
      }

      // lambda statement
      if (!strcmp(first->s, "lambda")) {
        if (length(args) < 2)
          evaluationError("not enough arguments for lambda");
        Item *closure = talloc(sizeof(Item));
        closure->type = CLOSURE_TYPE;
        closure->cl.frame = frame;
        closure->cl.paramNames = car(args);

        if (closure->cl.paramNames->type == CONS_TYPE) {
          // check for valid param formatting
          Item *checkParam = closure->cl.paramNames;
          while (checkParam->type == CONS_TYPE) {
            if (car(checkParam)->type != SYMBOL_TYPE)
              evaluationError("parameter must be symbol");

            // search prev symbols for duplicates
            Item *searchDup = cdr(checkParam);
            while (searchDup->type == CONS_TYPE) {
              if (!strcmp(car(checkParam)->s, car(searchDup)->s))
                evaluationError("duplicate parameter");
              searchDup = cdr(searchDup);
            }
            checkParam = cdr(checkParam);
          }
        } else if (closure->cl.paramNames->type != SYMBOL_TYPE &&
                   closure->cl.paramNames->type != NULL_TYPE) {
          evaluationError("parameters must be list");
        }

        closure->cl.functionCode = cdr(args);
        return closure;
      }

      // quote statement
      if (!strcmp(first->s, "quote")) {
        if (length(args) != 1)
          evaluationError("quote takes one argument");
        return car(args);
      }
    }

    // our symbol is not a special form so we'll evaluate it in the hopes its a
    // primitive or closure
    first = eval(first, frame);

    // apply closure
    if (first->type == CLOSURE_TYPE) {
      return apply(first, multiEval(args,frame));
    }

    // apply primitive
    if (first->type == PRIMITIVE_TYPE) {
      return eval(first, frame)->pf(multiEval(args,frame));
    }

    else {
      evaluationError("first thing in list wasn't a function or special form");
      /* // try to apply function */
      /* Item *evaledArgs = args; */
      /* while(args->type == CONS_TYPE) { */
      /*   args->c.car = eval(car(args),frame); */
      /*   args = cdr(args); */
      /* } */
      /* return apply(eval(first, frame), evaledArgs); */
      return makeNull();
    }
    break;
  }
  default: {
    evaluationError("default");
    return makeNull();
    break;
  }
  }
}
