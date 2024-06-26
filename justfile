SRCS := "linkedlist.c talloc.c main.c tokenizer.c parser.c interpreter.c"

CC := "clang"
CFLAGS := "-gdwarf-4 -fPIC"

default:
	just --list

build:
	{{CC}} {{CFLAGS}} {{SRCS}} -o interpreter
	rm -f *.o
	rm -f vgcore.*

compile target:
	{{CC}} {{CFLAGS}} -c {{target}} -o {{trim_end_match(target, ".c")}}-{{arch()}}.o

clean:
	-rm *.o
	-rm interpreter
