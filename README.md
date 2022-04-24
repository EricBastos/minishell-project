This is a project I made for the course CES-33 - Operating Systems.

It consists of a Minishell with piping capabilities, in which you can chain
as many programs as you want, while also being able to redirect both the input
and the output of these programs. You can also call any programs with arguments.

It follows the same syntax as the linux shell, so something like that would work:

cmd> ls | grep .c > out.txt
cmd> grep int < main.c

Simply run make all to build everything. Run ./minishell to execute.

Three helper functions to test the capabilities of the minishell are also included.
- ./print_argv arg1 arg2 ... : Prints "Print argv: arg1 arg2 ...
- ./print_hello : Prints "Print hello: Hello world"
- ./print_user : Reads stdin and prints "Print user: {message}"