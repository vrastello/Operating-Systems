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

char buffer_3[LINE_MAX * CHAR_MAX * sizeof(char)];
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
  while(strncmp(input, TERMINATOR, strlen(TERMINATOR)) != 0){
    strcpy(input, get_user_input());
    if(strncmp(input, TERMINATOR, strlen(input)) == 0){
      break;
    }
    put_buff_1(input);
    memset(input, '\0', CHAR_MAX);
  }
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
  while(strncmp(input, TERMINATOR, strlen(TERMINATOR)) != 0){
      strcpy(input, get_buff_1());
      if(strncmp(input, TERMINATOR, strlen(input)) == 0){
        break;
      }
      expand_input(input, "\n", " ");
      put_buff_2(input);
      memset(input, '\0', CHAR_MAX);
  }
  
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
  int len = strlen(input);
  strcpy(&buffer_3[prod_idx_3], input);
  // Increment the index where the next item will be put.
  prod_idx_3 = prod_idx_3 + len;
  count_3++;
}

void *plus_sign()
{
  char input[CHAR_MAX];
  memset(input, '\0', CHAR_MAX);
  while(strncmp(input, TERMINATOR, strlen(TERMINATOR)) != 0){
      strcpy(input, get_buff_2());
      if(strncmp(input, TERMINATOR, strlen(input)) == 0){
        break;
      }
      while(expand_input(input, "++", "^"));
      put_buff_3(input);
      memset(input, '\0', CHAR_MAX);
  }
  
  return NULL;
}

void *write_output()
{
  char output[OUT_MAX + 1];
}

int main()
{
  get_input();
  line_separator();
  plus_sign();
  printf("%s", buffer_3);
  printf("%d", strlen(buffer_3));
  
  return EXIT_SUCCESS;
}


