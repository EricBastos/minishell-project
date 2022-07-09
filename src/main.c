#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include "utils.h"


#define PIPELINE_SEPARATOR "|"
#define IN_SEPARATOR "<"
#define OUT_SEPARATOR ">"
#define ARGS_SEPARATOR " "

char** extractPipeOps(char* input, size_t* sz); // Extract {token1} | {token2} ....
void extractPathInputOutput(char* token, char** path, char** inputDir, char** outputDir); // Extract {path+args} > {out} < {in}
char** sepPathAndArgs(char* path, size_t* sz); // Extract {path} {arg1} {arg2} ....

// Node for each process
typedef struct programNode {
    char* programPath;
    char** argv;
    char* infile;
    char* outfile;
    int pipelineInput;
    int pipelineOutput;
} ProgramNode;

// Function to simulate C++ "new"
ProgramNode* New_ProgramNode(char* programPath, char** argv, char* infile, char* outfile) {
    ProgramNode* p = malloc(sizeof(ProgramNode));
    p->programPath = programPath;
    p->argv = argv;
    p->infile = infile;
    p->outfile = outfile;
    int pipeline[2];
    pipe(pipeline);
    p->pipelineInput = pipeline[1];
    p->pipelineOutput = pipeline[0];
    return p;
}

int main() {

    printf("Welcome to the miniature shell.\n");
    printf("Type exit to finish.\n");

    signal (SIGINT, SIG_IGN);
    signal (SIGQUIT, SIG_IGN);
    signal (SIGTSTP, SIG_IGN);
    signal (SIGTTIN, SIG_IGN);
    signal (SIGTTOU, SIG_IGN);
    signal (SIGCHLD, SIG_IGN);

    while (true) {
        printf("cmd> ");
        char* input = readString(stdin);

        if (strcmp(input, "exit") == 0) {
            printf("Exiting. Thanks for using the minishell.\n");
            break;
        }

        size_t tokenQty = 0;
        char** ops = extractPipeOps(input, &tokenQty);
        ProgramNode** nodes = realloc(NULL, sizeof(*nodes)*tokenQty);

        for (size_t i=0; i<tokenQty; i++) {

            char* path = NULL;
            char* inputDir = NULL;
            char* outputDir = NULL;

            extractPathInputOutput(ops[i], &path, &inputDir, &outputDir);
            
            size_t argc;
            char** pathAndArgs = sepPathAndArgs(path, &argc);

            nodes[i] = New_ProgramNode(pathAndArgs[0], pathAndArgs, inputDir, outputDir);

        }

        for (size_t i=0; i<tokenQty; i++) {

            pid_t child = fork();
            
            if (child == 0) { // Child code

                signal (SIGINT, SIG_DFL);
                signal (SIGQUIT, SIG_DFL);
                signal (SIGTSTP, SIG_DFL);
                signal (SIGTTIN, SIG_DFL);
                signal (SIGTTOU, SIG_DFL);
                signal (SIGCHLD, SIG_DFL);

                if (i == 0 && nodes[i]->infile != NULL) { // < is only accepted in the first process
                    int fd_id = open(nodes[i]->infile, O_RDONLY); // Set first program input to < file
                    dup2(fd_id, STDIN_FILENO);
                    close(fd_id);
                }

                if (i != tokenQty-1) { 
                    // Pipeline always receives the stdout, except for the last process 
                    // (which we may actually want to print to stdout)
                    dup2(nodes[i]->pipelineInput, STDOUT_FILENO);
                }

                if (nodes[i]->outfile != NULL) { // Redirect stdout to > file if that's the case
                    int fd_id = open(nodes[i]->outfile, O_RDWR | O_CREAT, 0666);
                    dup2(fd_id, STDOUT_FILENO);
                    close(fd_id);
                }

                if (i>0) { // Connect pipe[i-1] output to pipe[i] stdin
                    dup2(nodes[i-1]->pipelineOutput, STDIN_FILENO);
                }

                // Close all the channels we opened when creating the pipeline
                for (size_t tmp=0; tmp<tokenQty; tmp++) {
                    close(nodes[tmp]->pipelineInput);
                    close(nodes[tmp]->pipelineOutput);
                }

                execvp(nodes[i]->programPath, nodes[i]->argv);
                perror("execv()");
                _exit(255);
            } else if (child < 0) {
                printf("Fork failed, could not execute command.\n");
                exit(1);
            }
                
        }

        // Must now wait to free everything
        int wstatus;
        waitpid(-1, &wstatus, 0); // Wait all children to free resources
        
        // Close all the the channels and free *argv ptr
        for (size_t i=0; i<tokenQty; i++) {

            close(nodes[i]->pipelineInput);
            close(nodes[i]->pipelineOutput);
            
            free(nodes[i]->argv);
        }

        // Free all nodes and tokens and the input;
        free(nodes);
        free(ops);
        free(input);

    }
    
    return 0;

}

char** sepPathAndArgs(char* path, size_t* sz) {
    size_t argsCapacity = 2; // Capacity
    size_t argsSize = 0; // Current number of args

    char ** result = realloc(NULL, sizeof(*result)*argsCapacity);
    if(!result) return result; // Return if wasn't able to do so

    char* saveptr;
    char* token;

    token = strtok_r(path, ARGS_SEPARATOR, &saveptr);

    while (token != NULL) {
        if (argsSize == argsCapacity) { // Double capacity if necessary
            argsCapacity *= 2;
            result = realloc(result, sizeof(*result)*argsCapacity);
        }
        char* trimmedToken = trimWhiteSpace(token);
        result[argsSize++] = trimmedToken;
        token = strtok_r(NULL, ARGS_SEPARATOR, &saveptr);
    }
    result[argsSize++] = NULL; // Insert last arg as NULL, as requested by the documentation
    *sz = argsSize;
    return realloc(result, sizeof(*result)*argsSize); // Realloc to use the correct space
}

void extractPathInputOutput(char* token, char** path, char** inputDir, char** outputDir) {
    char* saveptrInput, *saveptrOutput;
    char* tokenInput, *tokenOutput;
    char* trimmedToken1, *trimmedToken2;
    tokenInput = strtok_r(token, IN_SEPARATOR, &saveptrInput);
    trimmedToken1 = trimWhiteSpace(tokenInput);

    tokenOutput = strtok_r(trimmedToken1, OUT_SEPARATOR, &saveptrOutput);
    trimmedToken2 = trimWhiteSpace(tokenOutput);
    // Trimmed2 should be path here

    *path = trimmedToken2;

    tokenOutput = strtok_r(NULL, OUT_SEPARATOR, &saveptrOutput);
    if (tokenOutput != NULL) { // Has out
        trimmedToken2 = trimWhiteSpace(tokenOutput);
        *outputDir = trimmedToken2;

        tokenOutput = strtok_r(NULL, OUT_SEPARATOR, &saveptrOutput); // Just to finish. Should be NULL now;
    }

    tokenInput = strtok_r(NULL, IN_SEPARATOR, &saveptrInput);
    if (tokenInput != NULL) { // Has in
        trimmedToken1 = trimWhiteSpace(tokenInput);

        // Try to divide on >
        tokenOutput = strtok_r(trimmedToken1, OUT_SEPARATOR, &saveptrOutput);
        trimmedToken2 = trimWhiteSpace(tokenOutput);
        *inputDir = trimmedToken1; // First should be in
        tokenOutput = strtok_r(NULL, OUT_SEPARATOR, &saveptrOutput);
        if (tokenOutput != NULL) {
            trimmedToken2 = trimWhiteSpace(tokenOutput);
            *outputDir = trimmedToken2; // Second should be out
        }
        
        tokenInput = strtok_r(NULL, IN_SEPARATOR, &saveptrInput); // Just to finish. Should be NULL now;
    }
}


char** extractPipeOps(char* input, size_t* sz) {

    size_t tokenCapacity = 2; // Capacity
    size_t tokenSize = 0; // Current number of tokens

    char ** result = realloc(NULL, sizeof(*result)*tokenCapacity);
    if(!result) return result; // Return if wasn't able to do so

    char* saveptr;
    char* token;

    token = strtok_r(input, PIPELINE_SEPARATOR, &saveptr);

    while (token != NULL) {
        if (tokenSize == tokenCapacity) { // Double capacity if necessary
            tokenCapacity *= 2;
            result = realloc(result, sizeof(*result)*tokenCapacity);
        }
        char* trimmedToken = trimWhiteSpace(token);
        result[tokenSize++] = trimmedToken;
        token = strtok_r(NULL, PIPELINE_SEPARATOR, &saveptr);
    }
    *sz = tokenSize;
    return realloc(result, sizeof(*result)*tokenSize); // Realloc to use the correct space
}