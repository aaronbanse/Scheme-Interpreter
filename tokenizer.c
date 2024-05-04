#include "tokenizer.h"
#include "linkedlist.h"
#include "talloc.h"
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define TOKEN_LEN_LIMIT 300

/* helper functions */
bool isInitial(char c) {
    return c=='!' || c=='$' || c=='%' || c=='&' || c=='*' || c=='/' || c==':'
        || c=='<' || c=='=' || c=='>' || c=='?' || c=='~' || c=='_' || c=='^'
        || isalpha(c);
}

bool isSymbol(char c) {
    return isInitial(c) || isdigit(c) || c=='.' || c=='+' || c=='-';
}

bool isDelim(char c) {
    return c==')' || c==']' || c==';' || c==EOF || isspace(c);
}

void error(char *message) {
    printf("Syntax error: %s\n", message);
    texit(1);
}

Item *tokenize() {
    int charRead;
    Item *list = makeNull();
    charRead = fgetc(stdin);
    while (charRead != EOF) {
        // search for non-whitespace
        if(isspace(charRead)) {
            charRead = fgetc(stdin);
            continue;
        }
        
        Item *token = makeNull();

        /* match open/close */
        if(charRead == '(' || charRead == ')' || charRead == '[' || charRead == ']') {
            switch (charRead) {
            case '(':
                token->type = OPEN_TYPE;
                break;
            case ')':
                token->type = CLOSE_TYPE;
                break;
            case '[':
                token->type = OPENBRACKET_TYPE;
                break;
            default:
                token->type = CLOSEBRACKET_TYPE;
                break;
            }
            token->s = talloc(sizeof(int)*2);
            sprintf(token->s, "%c", charRead);

        /* match comment */
        } else if(charRead == ';') {
            // follow comment till EOL or EOF, break if EOF
            while(charRead != '\n' && charRead != EOF) {
                charRead = fgetc(stdin);
            }
            if(charRead == EOF) break;
            continue;
        /* match number or + - sign */
        } else if(charRead == '+' || charRead == '-' || isdigit(charRead) || charRead == '.') {
            // initialize to unsigned
            int sign = 0;

            if(charRead == '+' || charRead == '-') {
                sign = charRead;
                charRead = fgetc(stdin);

                // check if +- is a symbol
                if(isDelim(charRead)) {
                    
                    token->type = SYMBOL_TYPE;
                    token->s = talloc(sizeof(int)*2);
                    sprintf(token->s, "%c", sign);

                    list = cons(token, list);
                    continue;
                }
            }

            char *num = talloc(sizeof(int)*(TOKEN_LEN_LIMIT + 1));
            if(sign) num[0] = sign;

            bool sawPoint = false;
            int i;

            for(i = (sign ? 1 : 0); i < TOKEN_LEN_LIMIT; i++) {
                if(isdigit(charRead)) {
                    num[i] = charRead;
                } else if(charRead == '.' && !sawPoint) {
                    num[i] = charRead;
                    sawPoint = true;
                } else {
                    num[i] = charRead;
                    num[i+1] = '\0';
                    error("invalid number");
                }

                charRead = fgetc(stdin);
                if(isDelim(charRead)) {
                    ungetc(charRead,stdin);
                    break;
                }
            }
            // need space for null terminator
            if(i == TOKEN_LEN_LIMIT) error("string longer than limit of TOKEN_LEN_LIMIT characters");

            // null terminate
            num[i+1] = '\0';

            // number can't consist of only decimal point
            if(i == (sign ? 1 : 0) && sawPoint) error("invalid number");

            if(sawPoint) {
                token->type = DOUBLE_TYPE;
                token->d = strtod(num, NULL);
            } else {
                token->type = INT_TYPE;
                token->i = strtol(num, NULL, 0);
            }

        /* match string */
        } else if (charRead == '\"') {
            token->type = STR_TYPE;
            token->s = talloc(sizeof(char)*(TOKEN_LEN_LIMIT + 1));

            int i = 0;
            while(i < TOKEN_LEN_LIMIT) { //fill in the temporary string
                token->s[i] = charRead;
                charRead = fgetc(stdin);
                i++;
                if(charRead == '\"' || charRead == EOF) break;//have we reached the end?
            }

            if(charRead == '\"') {
                token->s[i] = '\"';
            }
            token->s[i+1] = '\0'; //so we finish the string

            //token->s[TOKEN_LEN_LIMIT] = '\0';
            if(charRead == EOF) error("unexpected end of input while reading string");
            if(i == TOKEN_LEN_LIMIT) error("string longer than limit of TOKEN_LEN_LIMIT characters");

            charRead = fgetc(stdin);
            if(!isDelim(charRead)) error("No delimiter after string");
            ungetc(charRead, stdin);

        /* match symbol */
        } else if (isInitial(charRead)) {
            token->type = SYMBOL_TYPE;
            token->s = talloc(sizeof(char)*(TOKEN_LEN_LIMIT + 1));
            int i;

            for(i = 0; i < TOKEN_LEN_LIMIT; i++) {
                if (isSymbol(charRead)) {
                    token->s[i] = charRead;
                    token->s[i+1] = '\0'; //so we finish the symbol
                } else {
                    token->s[i] = charRead;
                    token->s[i+1] = '\0';
                    error("invalid symbol");
                }
                
                charRead = fgetc(stdin);
                if(isDelim(charRead)) {
                    ungetc(charRead, stdin);
                    break;
                }
            }

            if(i == TOKEN_LEN_LIMIT) error("string longer than limit of TOKEN_LEN_LIMIT characters");
            

        /* match boolean */
        } else if (charRead == '#') {
            token->type = BOOL_TYPE;
            charRead = fgetc(stdin);

            if(charRead == 'f') {
                token->i = 0;
            } else if(charRead == 't') {
                token->i = 1;
            } else {
                error("invalid boolean syntax");
            }

            charRead = fgetc(stdin);
            if(isDelim(charRead)) {
                ungetc(charRead, stdin);
            } else {
                error("invalid boolean syntax");
            }

        /* no match */
        } else {
            error("invalid token");
        }

        list = cons(token, list);
        charRead = fgetc(stdin);
    }

    Item *rev = reverse(list);
    return rev;
}

void displayTokens(Item *list) {
    while(list->type == CONS_TYPE) {
        Item *token = list->c.car;
        itemType t = token->type;
        if(t==OPEN_TYPE) {
            printf("(:open\n");
        } else if(t==CLOSE_TYPE) {
            printf("):close\n");
        } else if(t==OPENBRACKET_TYPE) {
            printf("[:openbracket\n");
        } else if(t==CLOSEBRACKET_TYPE) {
            printf("]:closebracket\n");
        } else if(t==SYMBOL_TYPE) {
            printf("%s:symbol\n", token->s);
        } else if(t==STR_TYPE) {
            printf("%s:string\n", token->s);
        } else if(t==BOOL_TYPE) {
            printf("#%c:boolean\n", token->i ? 't': 'f');
        } else if(t==INT_TYPE) {
            printf("%i:integer\n", token->i);
        } else if(t==DOUBLE_TYPE) {
            printf("%lf:double\n", token->d);
        }
        list = list->c.cdr;
    }
    printf("\n");
}