#include <stdio.h>
#include <ctype.h>
#include "tokenizer.h"
#include "linkedlist.h"
#include "talloc.h"
#include "stdbool.h"

#define TOKEN_LENGTH_LIMIT 300

//prints syntax error message then exits, freeing all memory used
void error(char *message) {
  printf("Syntax error: %s\n", message);
  texit(1);
}

//returns 1 if c is a delimiter in our grammar (if c could denote the end of a
//token) otherwise returns 0
int isDelim(char c) {
  return isspace(c)//checks if c is whitespace
    || c == ')'
    || c == ']'
    || c == ';'
    || c == EOF;
}

//returns 1 if c can be the start of a variable-length symbol in our grammar,
//otherwise return 0
int isInitial(char c) {//initials for symbols
  return isalpha(c) || c == '!' || c == '!' ||
    c == '$' || c == '%' || c == '&' ||
    c == '*' || c == '/' || c == ':' ||
    c == '<' || c == '=' || c == '>' ||
    c == '?' || c == '~' || c == '_' ||
    c == '^';
}


// Read all of the input from stdin, and returns a linked list where each car
// points to a token.
Item *tokenize() {
    int charRead;
    Item *list = makeNull();
    charRead = fgetc(stdin);

    while (charRead != EOF) {
      if(isspace(charRead)) {//if we encounter whitespace
        charRead = fgetc(stdin);//keep moving till it's something else
        continue;
      }
        //matching comments
      if (charRead == ';') {
        while(charRead != '\n' && charRead != EOF) {
          charRead=fgetc(stdin);
        }
      if(charRead==EOF) break;
      continue;
      }



      Item *token=makeNull();//there should be something to tokenize, lets make
                             //a placeholder for that token

      //matching opening and closing delims
      if (charRead == '(') {
        token->type=OPEN_TYPE;
        token->s=talloc(2*sizeof(char));
        sprintf(token->s,"%c",charRead);
      } else if (charRead == ')') {
        token->type=CLOSE_TYPE;
        token->s=talloc(2*sizeof(char));
        sprintf(token->s,"%c",charRead);
      } else if (charRead == '[') {
        token->type=OPENBRACKET_TYPE;
        token->s=talloc(2*sizeof(char));
        sprintf(token->s,"%c",charRead);
      } else if (charRead == ']') {
        token->type=CLOSEBRACKET_TYPE;
        token->s=talloc(2*sizeof(char));
        sprintf(token->s,"%c",charRead);

        //matching strings
      } else if (charRead == '\"') {
        token->type=STR_TYPE;
        char *temp_string=talloc(sizeof(char)*(TOKEN_LENGTH_LIMIT+1));
        int i=0;
        while(i<TOKEN_LENGTH_LIMIT) {//fill in the temporary string
          charRead=fgetc(stdin);
          if(charRead=='\"' || charRead ==EOF) break;//have we reached the end?
          temp_string[i]=charRead;
          i++;
        }
        if(charRead==EOF) error("unexpected end of input while reading string");
        if(i==TOKEN_LENGTH_LIMIT) error("string longer than limit of 300 characters");
        temp_string[i]='\0';//so we finish the string
        token->s=temp_string;
        charRead=fgetc(stdin);
        if(!isDelim(charRead)) error("No delimiter after string");
        ungetc(charRead,stdin);

        //matching booleans
      } else if (charRead == '#') {
        token->type=BOOL_TYPE;
        charRead=fgetc(stdin);
        if(charRead=='f') {
          token->i=0;
        } else if(charRead=='t') {
          token->i=1;
        } else {
          error("Invalid boolean syntax");
        }
        charRead=fgetc(stdin);
        if(!isDelim(charRead)) error("no delimiter after string");
        ungetc(charRead,stdin);

        // match number or + - sign
        } else if(charRead == '+' || charRead == '-' || isdigit(charRead) || charRead == '.') {
            // initialize to unsigned
            int sign = 0;

            if(charRead == '+' || charRead == '-') {
                sign = charRead;
                charRead = fgetc(stdin);

                // check if +- is a symbol
                if(isDelim(charRead) || charRead == EOF) {
                   
                    token->type = SYMBOL_TYPE;
                    token->s = talloc(sizeof(int)*2);
                    sprintf(token->s, "%c", sign);

                    list = cons(token, list);
                    continue;
                }
            }

            char *num = talloc(sizeof(char)*(TOKEN_LENGTH_LIMIT+1));
            if(sign) {
              num[0] = sign;//sign initialized to zero, then maybe changed to
                            //'+' or '-'. In that case we put it in num[0]
            }

            bool sawPoint = false;
            int i = (sign ? 1 : 0);
            while(i < TOKEN_LENGTH_LIMIT) {
                if(isdigit(charRead)) {
                    num[i] = charRead;
                } else if(charRead == '.' && !sawPoint) {
                    num[i] = charRead;
                    sawPoint = true;
                } else {
                    error("invalid characters in number");
                }
                charRead = fgetc(stdin);
                if(isDelim(charRead)) break;
                i++;
            }
            // currently charRead is delim or EOF. Step back so it will be the same at beginning of next loop
            ungetc(charRead, stdin);

            // need space for null terminator
            if(i == TOKEN_LENGTH_LIMIT) error("number exceeds character limit");

            // null terminate
            num[i+1] = '\0';

            // number can't consist of only decimal point
            if(i == (sign ? 1:0) && sawPoint)
              error("invalid number is just a decimal point");

            if(sawPoint) {
                token->type = DOUBLE_TYPE;
                token->d = strtod(num,NULL);
            } else {
                token->type = INT_TYPE;
                token->i = strtol(num,NULL,10);
            }

        //matching symbols (besides + or -)
      } else if (isInitial(charRead))//matching initials
        {
          token->type=SYMBOL_TYPE;
          char *temp_symbol=talloc(sizeof(char)*(TOKEN_LENGTH_LIMIT+1));
          int i;
          for(i=0; i<TOKEN_LENGTH_LIMIT; i++) {
            if(!(isInitial(charRead)
                 || isdigit(charRead)
                 || charRead=='.'
                 || charRead=='+'
                 || charRead=='-')) {
              error("invalid characters in symbol");
            }
            temp_symbol[i]=charRead;
            charRead=fgetc(stdin);
            if(isDelim(charRead)) {
              ungetc(charRead,stdin);
              break;
            }
          }
        if(i==TOKEN_LENGTH_LIMIT) error("symbol exceeds character limit");
        temp_symbol[i+1]='\0';//so we finish the symbol
        token->s=temp_symbol;
        } else error("invalid token");

      list=cons(token,list);
      charRead = fgetc(stdin);
    }


    Item *revList = reverse(list);
    return revList;
}

//prints the tokens stored in list to stdout, as value:type
void displayTokens(Item *list) {
  while(list->type==CONS_TYPE) {
    Item *token=list->c.car;
    if(token->type==STR_TYPE)
      printf("\"%s\":string\n",token->s);
    if(token->type==SYMBOL_TYPE)
      printf("%s:symbol\n",token->s);
    if(token->type==OPEN_TYPE)
      printf("%s:open\n",token->s);
    if(token->type==CLOSE_TYPE)
      printf("%s:close\n",token->s);
    if(token->type==OPENBRACKET_TYPE)
      printf("%s:openbracket\n",token->s);
    if(token->type==CLOSEBRACKET_TYPE)
      printf("%s:closebracket\n",token->s);
    if(token->type==INT_TYPE)
      printf("%i:integer\n",token->i);
    if(token->type==DOUBLE_TYPE)
      printf("%lf:double\n",token->d);
    if(token->type==BOOL_TYPE)
      printf("#%c:boolean\n",token->i? 't':'f');
    if(token->type==NULL_TYPE)
      printf("null:null\n");
    list=cdr(list);//move to next item in list
  }
}
