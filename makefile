# Minishell makefile
 
all: minishell print_hello print_user print_argv
 
minishell: main.o utils.o
	gcc -o minishell build/main.o build/utils.o
 
main.o: src/main.c src/utils.h
	gcc -o build/main.o src/main.c -c -W -Wall -pedantic
 
utils.o: src/utils.c src/utils.h
	gcc -o build/utils.o src/utils.c -c -W -Wall -pedantic

print_user: print_user.o utils.o
	gcc -o print_user build/print_user.o build/utils.o

print_user.o: src/helper_functions/print_user.c
	gcc -o build/print_user.o src/helper_functions/print_user.c -c -W -Wall -pedantic

print_argv: print_argv.o utils.o
	gcc -o print_argv build/print_argv.o build/utils.o

print_argv.o: src/helper_functions/print_argv.c
	gcc -o build/print_argv.o src/helper_functions/print_argv.c -c -W -Wall -pedantic

print_hello: print_hello.o utils.o
	gcc -o print_hello build/print_hello.o build/utils.o

print_hello.o: src/helper_functions/print_hello.c
	gcc -o build/print_hello.o src/helper_functions/print_hello.c -c -W -Wall -pedantic
 
clean:
	rm -rf *.o *~ minishell