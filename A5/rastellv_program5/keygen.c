#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

int main(int argc, char *argv[]) {
  
  // Check usage & args
  if (argc < 2) { 
    fprintf(stderr,"USAGE: %s keylength\n", argv[0]); 
    exit(0); 
  }
  char key_vals[28] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
  // set seed using time
  srand(time(0));
  //set keylength input from user including room for null terminator
  int size = atoi(argv[1]) + 1;
  char buffer[size];
  size_t len = strlen(key_vals);

  //create random string with keylength length
  for(int i = 0; i < size; i++){
    int rand_index = rand() % len;
    char rand_val = key_vals[rand_index];
    buffer[i] = rand_val;
  }
  //set null terminator and print
  buffer[size - 1] = '\0';
  printf("%s\n", buffer);
}
