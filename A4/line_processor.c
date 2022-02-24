#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h> 


#define LINE_MAX 50
#define CHAR_MAX 1000
#define OUT_MAX 80
#define TERMINATOR "STOP\n"

char buffer_1[LINE_MAX][CHAR_MAX];
int count_1 = 0;
int prod_idx_1 = 0;
int con_idx_1 = 0;
// Initialize the mutex for buffer 1
pthread_mutex_t mutex_1 = PTHREAD_MUTEX_INITIALIZER;
// Initialize the condition variable for buffer 1
pthread_cond_t full_1 = PTHREAD_COND_INITIALIZER;

char buffer_2[LINE_MAX][CHAR_MAX];
int count_2 = 0;
int prod_idx_2 = 0;
int con_idx_2 = 0;
// Initialize the mutex for buffer 2
pthread_mutex_t mutex_2 = PTHREAD_MUTEX_INITIALIZER;
// Initialize the condition variable for buffer 2
pthread_cond_t full_2 = PTHREAD_COND_INITIALIZER;

char buffer_3[LINE_MAX][CHAR_MAX];
int count_3 = 0;
int prod_idx_3 = 0;
int con_idx_3 = 0;
// Initialize the mutex for buffer 2
pthread_mutex_t mutex_3 = PTHREAD_MUTEX_INITIALIZER;
// Initialize the condition variable for buffer 2
pthread_cond_t full_3 = PTHREAD_COND_INITIALIZER;

char *get_user_input(){
  char *user_input = malloc(CHAR_MAX * sizeof(char));
  fgets(user_input, CHAR_MAX, stdin);
  return user_input;
}

/*
 Put an item in buff_1
*/
void put_buff_1(char *input){
  // Lock the mutex before putting the item in the buffer
  pthread_mutex_lock(&mutex_1);
  // Put the item in the buffer
  strcpy(&buffer_1[prod_idx_1][0], input);
  // Increment the index where the next item will be put.
  prod_idx_1 = prod_idx_1 + 1;
  count_1++;
  // Signal to the consumer that the buffer is no longer empty
  pthread_cond_signal(&full_1);
  // Unlock the mutex
  pthread_mutex_unlock(&mutex_1);
}

void *get_input(void *args)
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
  // Lock the mutex before checking if the buffer has data
  pthread_mutex_lock(&mutex_1);
  while(count_1 == 0){
    // Buffer is empty. Wait for the producer to signal that the buffer has data
    pthread_cond_wait(&full_1, &mutex_1);
  }
  char *buff_1_input = malloc(CHAR_MAX * sizeof(char));
  strcpy(buff_1_input, buffer_1[con_idx_1]);
  con_idx_1 = con_idx_1 + 1;
  count_1--;
  // Unlock the mutex
  pthread_mutex_unlock(&mutex_1);
  return buff_1_input;
}

void put_buff_2(char *input){
  // Lock the mutex before putting the item in the buffer
  pthread_mutex_lock(&mutex_2);
  // Put the item in the buffer
  strcpy(&buffer_2[prod_idx_2][0], input);
  // Increment the index where the next item will be put.
  prod_idx_2 = prod_idx_2 + 1;
  count_2++;
  // Signal to the consumer that the buffer is no longer empty
  pthread_cond_signal(&full_2);
  // Unlock the mutex
  pthread_mutex_unlock(&mutex_2);
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

void *line_separator(void *args)
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
  // Lock the mutex before checking if the buffer has data
  pthread_mutex_lock(&mutex_2);
  while(count_2 == 0){
    // Buffer is empty. Wait for the producer to signal that the buffer has data
    pthread_cond_wait(&full_2, &mutex_2);
  }
  char *buff_2_input = malloc(CHAR_MAX * sizeof(char));
  strcpy(buff_2_input, buffer_2[con_idx_2]);
  con_idx_2 = con_idx_2 + 1;
  count_2--;
  // Unlock the mutex
  pthread_mutex_unlock(&mutex_2);
  return buff_2_input;
}

void put_buff_3(char *input){
  // Lock the mutex before putting the item in the buffer
  pthread_mutex_lock(&mutex_3);
  // Put the item in the buffer
  strcpy(&buffer_3[prod_idx_3][0], input);
  // Increment the index where the next item will be put.
  prod_idx_3 = prod_idx_3 + 1;
  count_3++;
  // Signal to the consumer that the buffer is no longer empty
  pthread_cond_signal(&full_3);
  // Unlock the mutex
  pthread_mutex_unlock(&mutex_3);
}

void *plus_sign(void *args)
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
  // Lock the mutex before checking if the buffer has data
  pthread_mutex_lock(&mutex_3);
  while (count_3 == 0)
    // Buffer is empty. Wait for the producer to signal that the buffer has data
    pthread_cond_wait(&full_3, &mutex_3);
  char *buff_3_input = malloc(CHAR_MAX * sizeof(char));
  strcpy(buff_3_input, buffer_3[con_idx_3]);
  con_idx_3 = con_idx_3 + 1;
  count_3--;
  // Unlock the mutex
  pthread_mutex_unlock(&mutex_3);
  return buff_3_input;
}

void process_buff_3(char *large_string)
{
  //take input, separate out substring of 80, save remainder to current index, print substring
  printf("%.*s\n", OUT_MAX, large_string);
  memset(buffer_3[con_idx_3], '\0', CHAR_MAX);
  strcpy(&buffer_3[con_idx_3][0], large_string + OUT_MAX);
}

void *write_output(void *args)
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
  pthread_t input_t, line_separator_t, plus_sign_t, output_t;
  // Create the threads
  pthread_create(&input_t, NULL, get_input, NULL);
  pthread_create(&line_separator_t, NULL, line_separator, NULL);
  pthread_create(&plus_sign_t, NULL, plus_sign, NULL);
  pthread_create(&output_t, NULL, write_output, NULL);
  // Wait for the threads to terminate
  pthread_join(input_t, NULL);
  pthread_join(line_separator_t, NULL);
  pthread_join(plus_sign_t, NULL);
  pthread_join(output_t, NULL);

  return EXIT_SUCCESS;
}


