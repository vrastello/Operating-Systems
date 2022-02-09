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

// MAX_ARG_SIZE is to make array elements same size for indexing
#define MAX_ARG_SIZE 256
#define MAX_LINE 2048
#define MAX_ARGS 512
#define COMMENT "#"
#define EXPAND "$$"
#define JOB_ID_SIZE 100

/* struct for command information */
struct command
{
    char *comm;
    char *args[512];
    char *input;
    char *output;
    bool *background;
};

//iterate to last value and check for &, set background to true if found
struct command *processArgs(struct command *currCommand)
{
  int i = 0;
  char *temp = NULL;
  bool *val = true;

  //iterate to last value
  while(currCommand->args[i] != NULL && i < MAX_ARGS){
    temp = currCommand->args[i];
    i++;
  }
  // check if last value is &, and set background to true if so
  if(temp != NULL && (strncmp(temp, "&", strlen(temp)) == 0)){
    currCommand->background = calloc(1, 10);
    currCommand->background = val;
    //remove & from args array
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

    // add command to list to make execvp function easier
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
        //increment token
        token = strtok_r(NULL, " \n", &saveptr);
        // save next arg as input
        currCommand->input = calloc(strlen(token) + 1, sizeof(char));
        strcpy(currCommand->input, token);
      }
      //save output
      else if(strncmp(token, ">", strlen(token)) == 0){
        //increment token
        token = strtok_r(NULL, " \n", &saveptr);
        // save next arg as ouput
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

    // if at least one arg
    if(n > 0){
      processArgs(currCommand);
    }

    return currCommand;
}

void processExit()
{

}

void changeDirectory(struct command *currCommand)
{
  char *path;
  char curr[1048];
  char rootPath[1048] = "";

  // check for arg sent to cd, if none go to env HOME
  if (currCommand->args[1] == NULL){
    path = getenv("HOME");
    chdir(path);
  }

  // else take arg as path
  else{
    path = currCommand->args[1];
    snprintf(rootPath, sizeof(rootPath), "%s", path);
    chdir(rootPath);
  }
}

void status(int currChildStatus, bool normalExit)
// print status of last completed task
{
  if(normalExit){
    printf("exit value %d\n", currChildStatus);
  }
  else{
    printf("terminated by signal %d\n", currChildStatus);
  }
}

void backgroundStatus(int currChildStatus, bool normalExit, pid_t pid)
// print status of last completed task
{
  if(normalExit){
    printf("background pid %d is done: exit value %d\n", pid, currChildStatus);
  }
  else{
    printf("background pid %d is done: terminated by signal %d\n", pid, currChildStatus);
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
    int  childStatus;
    int  currChildStatus = 0;
    bool normalExit = true;
    int  backgroundPids[100];
    memset(backgroundPids, 0, sizeof(backgroundPids));
    int  arrPosition = 0;

    int pid = getpid();
    char parentJobId[JOB_ID_SIZE];
    // convert pid to string for expansion
    snprintf(parentJobId, JOB_ID_SIZE, "%d", pid);

    // prompts for for user
    while(true){

        // check for completed background jobs, and clean up zombies, print job completion
        int j = 0;
        for(j = 0; j < 100; j++){

          if(backgroundPids[j] != 0){
            pid_t backChildPid = waitpid(backgroundPids[j], &childStatus, WNOHANG);

            if(backChildPid != 0){
              arrPosition -= 1;
              if(WIFEXITED(childStatus)){
                normalExit = true;
                currChildStatus = WEXITSTATUS(childStatus);
              } 
              else{
                normalExit = false;
                currChildStatus = WTERMSIG(childStatus);
              }
              backgroundStatus(currChildStatus, normalExit, backgroundPids[j]);
              fflush(stdout);
              memset(&backgroundPids[j], 0, sizeof(int));
              waitpid(backgroundPids[j], &childStatus, WNOHANG);
            }
          }
        }

        //reset background job array position for new background jobs to be added 
        printf("%d\n", j);
        printf("%d\n", arrPosition);

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
        // check_errors(user_cmd, pid);

        //cd
        if(strncmp(user_cmd->comm, "cd", strlen(user_cmd->comm)) == 0){
          changeDirectory(user_cmd);
          continue;
        }

        //status
        if(strncmp(user_cmd->comm, "status", strlen(user_cmd->comm)) == 0){
          status(currChildStatus, normalExit);
          fflush(stdout);
          continue;
        }
        //other commands
        else{
          pid_t childPid = fork();

          if(childPid == -1){
            perror("fork() failed!");
            exit(1);
          } else if(childPid == 0){
            // restructure new array to be compatible with execvp function
            int i = 0;
            //iterate to last value to get size of array
            while(user_cmd->args[i] != NULL && i < MAX_ARGS){
              i++;
            }
            // make new array
            int arrSize = i + 1;
            char *newArr[arrSize];
            i = 0;
            while(i <= arrSize){
              if(i < arrSize - 1){
                newArr[i] = calloc(MAX_ARG_SIZE, sizeof(char));
                strcpy(newArr[i], user_cmd->args[i]);
              }
              // add null to last value of arr
              else{
                newArr[i] = NULL;
              }
              i++;
            }

            // large portions of this were taken from Module 5 exploration processes and I/O
            if(user_cmd->input != NULL){
              //open source file
              int sourceFD = open(user_cmd->input, O_RDONLY);
              if (sourceFD == -1) { 
                perror("source open()"); 
                exit(1); 
	            }

              // Redirect stdin to source file
              int result = dup2(sourceFD, 0);
              if (result == -1) { 
                perror("source dup2()"); 
                exit(2); 
              }
            }

            if(user_cmd->output != NULL){
              // Open target file
              int targetFD = open(user_cmd->output, O_WRONLY | O_CREAT | O_TRUNC, 0777);
              if (targetFD == -1) { 
                perror("target open()"); 
                exit(1); 
              }
              
              // Redirect stdout to target file
              int result = dup2(targetFD, 1);
              if (result == -1) { 
                perror("target dup2()"); 
                exit(2); 
              }
            }

            // Child process
            execvp(user_cmd->comm, newArr);
            // if execvp error return exit val 1, setting status to 1
            perror("execvp");
            exit(1);
            break;

          // parent process
          } else{
              if(user_cmd->background){
                backgroundPids[arrPosition] = childPid;
                arrPosition += 1;
                printf("background pid is %d\n", childPid);
                fflush(stdout);
                childPid = waitpid(childPid, &childStatus, WNOHANG);
              }
              else{
                childPid = waitpid(childPid, &childStatus, 0);
              }
              // set variables for built in status function
              if(WIFEXITED(childStatus)){
                normalExit = true;
                currChildStatus = WEXITSTATUS(childStatus);
              } 
              else{
                normalExit = false;
                currChildStatus = WTERMSIG(childStatus);
              }
          }
        }
    }
    processExit();
    return 0;
}
