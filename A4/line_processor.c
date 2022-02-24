#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h> // must link with -lm

/*
A program with a pipeline of 3 threads that interact with each other as producers and consumers.
- Input thread is the first thread in the pipeline. It gets input from the user and puts it in a buffer it shares with the next thread in the pipeline.
- Square root thread is the second thread in the pipeline. It consumes items from the buffer it shares with the input thread. It computes the square root of this item. It puts the computed value in a buffer it shares with the next thread in the pipeline. Thus this thread implements both consumer and producer functionalities.
- Output thread is the third thread in the pipeline. It consumes items from the buffer it shares with the square root thread and prints the items.
*/

#define LINE_MAX 50
#define CHAR_MAX 1000
#define OUT_MAX 80
#define TERMINATOR "STOP\n"

char buffer_1[LINE_MAX][CHAR_MAX];
int count_1 = 0;
int prod_idx_1 = 0;
int con_idx_1 = 0;

char buffer_2[LINE_MAX][CHAR_MAX];
int count_2 = 0;
int prod_idx_2 = 0;
int con_idx_2 = 0;

char buffer_3[LINE_MAX][CHAR_MAX];
int count_3 = 0;
int prod_idx_3 = 0;
int con_idx_3 = 0;

char *get_user_input(){
  char *user_input = malloc(CHAR_MAX * sizeof(char));
  fgets(user_input, CHAR_MAX, stdin);
  return user_input;
}

/*
 Put an item in buff_1
*/
void put_buff_1(char *input){
  // Put the item in the buffer
  strcpy(&buffer_1[prod_idx_1][0], input);
  // Increment the index where the next item will be put.
  prod_idx_1 = prod_idx_1 + 1;
  count_1++;
}

void *get_input()
{
  char input[CHAR_MAX];
  memset(input, '\0', CHAR_MAX);
  strcpy(input, get_user_input());
  while(strncmp(input, TERMINATOR, strlen(TERMINATOR)) != 0){
    put_buff_1(input);
    memset(input, '\0', CHAR_MAX);
    strcpy(input, get_user_input());
  }

  put_buff_1(input);
  return NULL;
}

char *get_buff_1(){
  char *buff_1_input = malloc(CHAR_MAX * sizeof(char));
  strcpy(buff_1_input, buffer_1[con_idx_1]);
  con_idx_1 = con_idx_1 + 1;
  count_1--;
  return buff_1_input;
}

void put_buff_2(char *input){
  // Put the item in the buffer
  strcpy(&buffer_2[prod_idx_2][0], input);
  // Increment the index where the next item will be put.
  prod_idx_2 = prod_idx_2 + 1;
  count_2++;
}

char* expand_input(char* input, char* target, char* replacement) {
    //parts of this code were inspired from my code from project 3

    //find first instance of target, sets pointer to that part of string
    char* input_pointer = strstr(input, target);
    //if none found return null breaks loop
    if (input_pointer == NULL) {
        return NULL;
    }

    //inserts additonal memory into string at substring_source pointer to expand for replacement
    memmove(
        input_pointer + strlen(replacement),
        input_pointer + strlen(target),
        strlen(input_pointer) - strlen(target) + 1
    );

    //copies replacement starting at pointer where first instance of target starts
    memcpy(input_pointer, replacement, strlen(replacement));

    // returns pointer to where replacement string ends to run code on the rest of the string
    return input_pointer + strlen(replacement);
}

void *line_separator()
{
  char input[CHAR_MAX];
  memset(input, '\0', CHAR_MAX);
  strcpy(input, get_buff_1());
  while(strncmp(input, TERMINATOR, strlen(TERMINATOR)) != 0){
      expand_input(input, "\n", " ");
      put_buff_2(input);
      memset(input, '\0', CHAR_MAX);
      strcpy(input, get_buff_1());
  }
  put_buff_2(input);
  return NULL;
}

char *get_buff_2(){
  char *buff_2_input = malloc(CHAR_MAX * sizeof(char));
  strcpy(buff_2_input, buffer_2[con_idx_2]);
  con_idx_2 = con_idx_2 + 1;
  count_2--;
  return buff_2_input;
}

void put_buff_3(char *input){
  // Put the item in the buffer
  strcpy(&buffer_3[prod_idx_3][0], input);
  // Increment the index where the next item will be put.
  prod_idx_3 = prod_idx_3 + 1;
  count_3++;
}

void *plus_sign()
{
  char input[CHAR_MAX];
  memset(input, '\0', CHAR_MAX);
  strcpy(input, get_buff_2());
  while(strncmp(input, TERMINATOR, strlen(TERMINATOR)) != 0){
      while(expand_input(input, "++", "^"));
      put_buff_3(input);
      memset(input, '\0', CHAR_MAX);
      strcpy(input, get_buff_2());
  }
  put_buff_3(input);
  return NULL;
}

char *get_buff_3(){
  char *buff_3_input = malloc(CHAR_MAX * sizeof(char));
  strcpy(buff_3_input, buffer_3[con_idx_3]);
  con_idx_3 = con_idx_3 + 1;
  count_3--;
  return buff_3_input;
}

void process_buff_3(char *large_string)
{
  //take input, separate out substring of 80, save remainder to current index, print substring
  printf("%.*s\n", OUT_MAX, large_string);
  memset(buffer_3[con_idx_3], '\0', CHAR_MAX);
  strcpy(&buffer_3[con_idx_3][0], large_string + OUT_MAX);
}

void *write_output()
{
  char *input = malloc(CHAR_MAX * sizeof(char));
  memset(input, '\0', CHAR_MAX);
  strcpy(input, get_buff_3());
  while(strncmp(input, TERMINATOR, strlen(TERMINATOR)) != 0){
      // put code here LOL
      if(strlen(input) >= OUT_MAX){
        if(strlen(input) == OUT_MAX){
          printf("%s\n", input);
        }
        else{
          count_3++;
          con_idx_3 = con_idx_3 - 1;
          process_buff_3(input);
        }
      }
      else{
        char *curr_str = malloc(CHAR_MAX * 2 * sizeof(char));
        char next_str[CHAR_MAX];
        strcpy(curr_str, input);
        while(strlen(curr_str) < OUT_MAX){
          strcpy(next_str, get_buff_3());
          if (strncmp(next_str, TERMINATOR, strlen(TERMINATOR)) == 0){
            return NULL;
          }
          strcat(curr_str, next_str);
          memset(next_str, '\0', CHAR_MAX);
        }
        if(strlen(curr_str) == OUT_MAX){
          printf("%s\n", input);
        }
        else{
          count_3++;
          con_idx_3 = con_idx_3 - 1;
          process_buff_3(curr_str);
        }
      }
      memset(input, '\0', CHAR_MAX);
      strcpy(input, get_buff_3());
  }
  return NULL;
}

int main()
{
  get_input();
  line_separator();
  plus_sign();
  write_output();

  return EXIT_SUCCESS;
}


