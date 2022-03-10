#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <ctype.h> 
#include <stdbool.h>
#include <sys/types.h>  // ssize_t
#include <sys/stat.h>
#include <sys/socket.h> // send(),recv()
#include <netdb.h>      // gethostbyname()

#define SEG_SIZE 1000
#define CLIENT_ID "enc_client"
#define CONN_SUCCESS "CLIENT ID CONFIRMED"

/**
* Client code
* 1. Create a socket and connect to the server specified in the command arugments.
* 2. Prompt the user for input and send that input as a message to the server.
* 3. Print the message received from the server and exit the program.
*/

// Error function used for reporting issues
void error(const char *msg) { 
  perror(msg); 
  exit(1); 
} 

// Set up the address struct
void setupAddressStruct(struct sockaddr_in* address, 
                        int portNumber){
 
  // Clear out the address struct
  memset((char*) address, '\0', sizeof(*address)); 

  // The address should be network capable
  address->sin_family = AF_INET;
  // Store the port number
  address->sin_port = htons(portNumber);

  char hostname[] = "localhost";
  // Get the DNS entry for this host name
  struct hostent* hostInfo = gethostbyname(hostname); 
  if (hostInfo == NULL) { 
    fprintf(stderr, "CLIENT: ERROR, no such host\n"); 
    exit(0); 
  }
  // Copy the first IP address from the DNS entry to sin_addr.s_addr
  memcpy((char*) &address->sin_addr.s_addr, 
        hostInfo->h_addr_list[0],
        hostInfo->h_length);
}

void checkBadInputs(int fd, off_t fsize)
{
  //check for bad inputs in plain file
  char input_read[fsize];
  char valid_keys[28] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
  read(fd, input_read, sizeof(input_read));
  bool success;
  for(int i = 0; i < fsize - 1; i++){
    success = false;
    for(int j = 0; j < sizeof(valid_keys) - 1; j++){
      char temp_key = valid_keys[j];
      char temp_read = toupper(input_read[i]);
      if(strncmp(&temp_read, &temp_key, 1) == 0){
        success = true;
        break;
      } 
    }
    if(success == false){
      fprintf(stderr, "Client Error: input contains bad characters\n");
      exit(1);
    }
  }
  lseek(fd, 0, SEEK_SET);
}

int main(int argc, char *argv[]) {
  int socketFD, portNumber, charsWritten, charsRead;
  struct sockaddr_in serverAddress;
  // Check usage & args
  if (argc < 4) { 
    fprintf(stderr,"USAGE: %s plaintext key port\n", argv[0]); 
    exit(0); 
  } 

  // Create a socket
  socketFD = socket(AF_INET, SOCK_STREAM, 0); 
  if (socketFD < 0){
    error("CLIENT: ERROR opening socket");
  }

   // Set up the server address struct
  setupAddressStruct(&serverAddress, atoi(argv[3]));

  // Connect to server
  if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
    error("CLIENT: ERROR connecting");
  }

  int segment_size = SEG_SIZE;
  char readBuffer[segment_size];
  char writeBuffer[segment_size];

  //send identifier to server
  memset(readBuffer, 0, sizeof(readBuffer));
  send(socketFD, CLIENT_ID, strlen(CLIENT_ID), 0);
  recv(socketFD, readBuffer, sizeof(readBuffer), 0);
  if(strncmp(readBuffer, CONN_SUCCESS, strlen(readBuffer)) != 0){
    fprintf(stderr, "Client Error: could not contact enc_server on port %d\n", ntohs(serverAddress.sin_port));
    exit(2);
  }
  
  printf("CLIENT: I received this from the server: \"%s\"\n", readBuffer);

  //open plainfile
  char* plain = argv[1];
  int plain_fd;
  plain_fd = open(plain, O_RDONLY, 00600);
  if (plain_fd == -1){
    error("Failed to open plainfile for reading\n");
  }
  //open key file
  char* key = argv[2];
  int key_fd;
  key_fd = open(key, O_RDONLY, 00600);
  if(key_fd == -1){
    error("Failed to open keyfile for reading\n");
  }
  //get file size of key file
  struct stat key_buf;
  fstat(key_fd, &key_buf);
  off_t key_size = key_buf.st_size;
  printf("THIS IS KEY FILE SIZE: %d\n", key_size);

  //get file size of plain file
  struct stat plain_buf;
  fstat(plain_fd, &plain_buf);
  off_t plain_size = plain_buf.st_size;
  printf("THIS IS PLAIN FILE SIZE: %d\n", plain_size);

  //check if key and plain file same size
  if(plain_size != key_size){
    fprintf(stderr, "Client Error: key file size does not equal plain file size\n");
    exit(1);
  }

  //check for bad inputs in both files
  checkBadInputs(plain_fd, plain_size);
  checkBadInputs(key_fd, key_size);



  // send in 1000 char increments or "chunks", rounding up
  int num_chunk = ((plain_size - 1) + (SEG_SIZE - 1)) / SEG_SIZE;

  while (num_chunk != 0)
  {
    read(plain_fd, writeBuffer, sizeof(writeBuffer));
    // Remove the trailing \n 
    writeBuffer[strcspn(writeBuffer, "\n")] = '\0'; 

    // Send message to server
    // Write to the server
    size_t buf_len_bytes = strlen(writeBuffer);
    size_t c_written = 0;
    //to handle in case data is lost
    while(c_written < buf_len_bytes){
      ssize_t response = send(socketFD, ((char *) writeBuffer) + c_written, buf_len_bytes - c_written, 0); 
      if (charsWritten < 0){
        error("CLIENT: ERROR writing to socket");
      }
      else{
        c_written += response;
      }
    }

    // Get return message from server
    // Clear out the buffer again for reuse
    memset(readBuffer, 0, sizeof(readBuffer));
    // Read data from the socket, leaving \0 at end
    charsRead = recv(socketFD, readBuffer, sizeof(readBuffer), 0); 
    if (charsRead < 0){
      error("CLIENT: ERROR reading from socket");
    }
    printf("CLIENT: I received this from the server: \"%s\"\n", readBuffer);
    printf("chars read: %d", charsRead);
    num_chunk--;
  }
  close(plain_fd);
  // Close the socket
  close(socketFD);
  return 0;
}