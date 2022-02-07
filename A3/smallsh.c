// If you are not compiling with the gcc option --std=gnu99, then
// uncomment the following line or you might get a compiler warning
//#define _GNU_SOURCE

/* 
SOURCES:
 C tutorial https://www.youtube.com/watch?v=0qSU0nxIZiE
 All explorations in module 4 and 5
*/

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>

// MAX_ARG_SIZE is to make array elements same size for simpler implementation
#define MAX_ARG_SIZE 50
#define MAX_LINE 2048
#define MAX_ARGS 512
#define COMMENT "#"
#define EXPAND "$$"
#define JOB_ID_SIZE 20

/* struct for command information */
struct command
{
    char *comm;
    char *args[MAX_ARGS];
    char *input;
    char *output;
    bool *background;
};

//iterate to last value and check for &, set background to true if found
struct command *processCommand(struct command *currCommand)
{
  int i = 0;
  char *temp = NULL;
  bool *val = true;

  //iterate to last value
  while(currCommand->args[i] != NULL && i < MAX_ARGS){
    temp = currCommand->args[i];
    i++;
  }
  // check if last value is & and set last value to null if so
  if(temp != NULL && (strncmp(temp, "&", strlen(temp)) == 0)){
    currCommand->background = calloc(1, 10);
    currCommand->background = val;
    memset(currCommand->args + (i - 1), '\0', MAX_ARG_SIZE);
  }

  return currCommand;
}

// /* parse command into struct */
struct command *parseCommand(char *input)
{
    struct command *currCommand = malloc(sizeof(struct command));
    char *saveptr;
    int n = 0;
    int i = 0;

    // The first token is the command
    char *token = strtok_r(input, " \n", &saveptr);
    currCommand->comm = calloc(strlen(token) + 1, sizeof(char));
    strcpy(currCommand->comm, token);

    // add command to list to make exec function easier
    currCommand->args[i] = calloc(MAX_ARG_SIZE, sizeof(char));
    strcpy(currCommand->args[i], token);
    i++;

    // The next token is the array of args
    token = strtok_r(NULL, " \n", &saveptr);

    //loop through args and save to array, max args are 512
    while (token != NULL && n < MAX_ARGS)
    {
      //save input
      if(strncmp(token, "<", strlen(token)) == 0){
        currCommand->input = calloc(strlen(token) + 1, sizeof(char));
        strcpy(currCommand->input, token);
      }
      //save output
      else if(strncmp(token, ">", strlen(token)) == 0){
        currCommand->output = calloc(strlen(token) + 1, sizeof(char));
        strcpy(currCommand->output, token);
      }
      //save to arguments array
      else{
        currCommand->args[i] = calloc(MAX_ARG_SIZE, sizeof(char));
        strcpy(currCommand->args[i], token);
        i++;
      }
      //increment token
      token = strtok_r(NULL, " \n", &saveptr);
      n++;
    }

    processCommand(currCommand);

    return currCommand;
}

void processExit()
{

}

//frees all memory allocated to command and it's members
void freeAll(struct command *currCommand)
{

}

void changeDirectory(struct command *currCommand)
{
  char *path;
  char curr[1048];
  char rootPath[1048] = "";

  if (currCommand->args[0] == NULL){
    path = getenv("HOME");
    chdir(path);
  }

  else{
    path = currCommand->args[0];
    snprintf(rootPath, sizeof(rootPath), "./%s", path);
    chdir(rootPath);
  }

  printf("%s", getcwd(curr, 256));
}

void status(int childStatus, bool normalExit)
{
  if(normalExit){
    printf("exit value %d\n", childStatus);
  }
  else{
    printf("terminated by signal %d\n", childStatus);
  }
}

void check_errors(struct command *user_cmd, int pid)
{
  printf("%d ", pid);
  printf("%s ", user_cmd->comm);
  printf("%s ", user_cmd->input);
  printf("%s ", user_cmd->output);
  if(user_cmd->background){
    printf("background is true\n");
  }     

  int i = 0;
  while(user_cmd->args[i] != NULL){
    printf("%s ", user_cmd->args[i]);
    i++;
  }
}

char* expand_input(char* input, char* expand, char* parentJobId) {
    //parts of this code were inspired from c tutorial in sources

    //find first instance of $$, sets pointer to that part of string
    char* input_pointer = strstr(input, expand);
    //if none found return null breaks loop
    if (input_pointer == NULL) {
        return NULL;
    }

    //inserts additonal memory into string at substring_source pointer to expand parentJobId
    memmove(
        input_pointer + strlen(parentJobId),
        input_pointer + strlen(expand),
        strlen(input_pointer) - strlen(expand) + 1
    );

    //copies parentjobId starting at pointer where first instance of $$ starts
    memcpy(input_pointer, parentJobId, strlen(parentJobId));

    // returns pointer to where parentjobID ends to run code on the rest of the string
    return input_pointer + strlen(parentJobId);
}

int main(int argc)
{
    char input[MAX_LINE];
    char temp[MAX_LINE];
    int  childStatus = 0;
    bool normalExit = true;
    int pid = getpid();
    char parentJobId[JOB_ID_SIZE];
    // convert pid to string for expansion
    snprintf(parentJobId, JOB_ID_SIZE, "%d", pid);

    // prompts for for user
    while(true){

        printf(": ");
        fgets(input, MAX_LINE, stdin);

        strcpy(temp, input);
        char *token = strtok(temp, " \n");

        //if nothing entered by user
        if(token == NULL){
          continue;
        }
        // if comment entered
        if(strncmp(token, COMMENT, strlen(COMMENT)) == 0){
          continue;
        }
        //exit
        if(strncmp(token, "exit", strlen(token)) == 0){
          break;
        }

        //expand variable $$, loop to expand every instance
        while(expand_input(input, EXPAND, parentJobId));

        //usable input
        struct command *user_cmd = parseCommand(input);
        check_errors(user_cmd, pid);

        //cd
        if(strncmp(user_cmd->comm, "cd", strlen(user_cmd->comm)) == 0){
          changeDirectory(user_cmd);
        }

        //status
        if(strncmp(user_cmd->comm, "status", strlen(user_cmd->comm)) == 0){
          status(childStatus, normalExit);
          fflush(stdout);
        }
        //other commands
        else{
          pid_t childPid = fork();

          if(childPid == -1){
            perror("fork() failed!");
            exit(1);
          } else if(childPid == 0){
            // Child process
            sleep(10);
            exit(0);
          } else{
            printf("Child's pid = %d\n", childPid);
            childPid = waitpid(childPid, &childStatus, 0);
            printf("waitpid returned value %d\n", childPid);
            // set variables for built in status function
            if(WIFEXITED(childStatus)){
              normalExit = true;
              childStatus = WEXITSTATUS(childStatus);
            } 
            else{
              normalExit = false;
              childStatus = WTERMSIG(childStatus);
            }
          }
        }
    }
    processExit();
    return 0;
}
