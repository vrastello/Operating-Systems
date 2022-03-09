#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/types.h>  // ssize_t
#include <sys/socket.h> // send(),recv()
#include <netdb.h>      // gethostbyname()

#define SEG_SIZE 4

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
  //send plainfile
  char* plain = argv[1];
  char* key = argv[2];
  int plain_fd;
  plain_fd = open(plain, O_RDONLY, 00600);
  if (plain_fd == -1){
    error("Failed to open plainfile for reading\n");
  }

  int file_pointer = 0;
  int segment_size = SEG_SIZE;
  char readBuffer[segment_size];
  char writeBuffer[segment_size];
  ssize_t nread;
  // read in 1000 char increments
  int count = 0;

  while (count < 10)
  {
      nread = read(plain_fd, writeBuffer, sizeof(writeBuffer));
    // Remove the trailing \n that fgets adds
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
    count++;
  }
  close(plain_fd);
  // Close the socket
  close(socketFD);
  return 0;
}