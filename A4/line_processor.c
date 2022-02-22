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

#define L_MAX 50
#define C_MAX 1000
#define NUM_L = 49
#define TERMINATOR "STOP\n"

char buffer_1[L_MAX][C_MAX];
int count_1 = 0;
int prod_idx_1 = 0;
int con_idx_1 = 0;

char buffer_2[L_MAX][C_MAX];
int count_2 = 0;
int prod_idx_2 = 0;
int con_idx_2 = 0;

char buffer_3[L_MAX][C_MAX];
int count_3 = 0;
int prod_idx_3 = 0;
int con_idx_3 = 0;

char *get_user_input(){
  char *user_input = malloc(C_MAX * sizeof(char));
  fgets(user_input, C_MAX, stdin);
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
  char input[C_MAX];
  while(strncmp(input, TERMINATOR, strlen(TERMINATOR)) != 0){
    strcpy(input, get_user_input());
    if(strncmp(input, TERMINATOR, strlen(input)) == 0){
      break;
    }
    put_buff_1(input);
  }
  return NULL;
}

char *get_buff_1(){
  char input[] = buffer_1[con_idx_1];
  con_idx_1 = con_idx_1 + 1;
  count_1--;
  return input;
}

void put_buff_2(char *input){
  // Put the item in the buffer
  strcpy(&buffer_2[prod_idx_2][0], input);
  // Increment the index where the next item will be put.
  prod_idx_2 = prod_idx_2 + 1;
  count_2++;
}

void *line_separator()
{
  
}

int main()
{
  get_input();
  for(int i = 0; i < L_MAX; i++){
    printf("%s", buffer_1[i]);
  }
  return EXIT_SUCCESS;
}


