// If you are not compiling with the gcc option --std=gnu99, then
// uncomment the following line or you might get a compiler warning
//#define _GNU_SOURCE

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>

#define MAX_ARG_SIZE 50
#define MAX_LINE 2048
#define MAX_ARGS 512
#define comment "#"

/* struct for command information */
struct command
{
    char *comm;
    // array of pointers to args 
    char *args[MAX_ARGS];
    char *input;
    char *output;
    bool *background;
};

//iterate to last value and check for &, set background if so
struct command *processCommand(struct command *currCommand)
{
  //iterate to last value
  int i = 0;
  char *temp = NULL;
  bool *val = true;

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

int promptUser()
{
    char input[MAX_LINE];
    char temp[MAX_LINE];

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
        if(strncmp(token, comment, strlen(comment)) == 0){
          continue;
        }
        //exit
        if(strncmp(token, "exit", strlen(token)) == 0){
          break;
        }

        struct command *user_cmd = parseCommand(input);

        if(strncmp(user_cmd->comm, "cd", strlen(user_cmd->comm)) == 0){
          changeDirectory(user_cmd);
        }

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
    processExit();
    return 0;
}

int main(int argc)
{
    promptUser();
}