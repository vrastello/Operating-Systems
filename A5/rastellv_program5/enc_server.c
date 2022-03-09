#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>

// Error function used for reporting issues
void error(const char *msg) {
  perror(msg);
  exit(1);
} 

// Set up the address struct for the server socket
void setupAddressStruct(struct sockaddr_in* address, 
                        int portNumber){
 
  // Clear out the address struct
  memset((char*) address, '\0', sizeof(*address)); 

  // The address should be network capable
  address->sin_family = AF_INET;
  // Store the port number
  address->sin_port = htons(portNumber);
  // Allow a client at any address to connect to this server
  address->sin_addr.s_addr = INADDR_ANY;
}

int main(int argc, char *argv[]){
  int connectionSocket, charsRead;
  struct sockaddr_in serverAddress, clientAddress;
  socklen_t sizeOfClientInfo = sizeof(clientAddress);

  // Check usage & args
  if (argc < 2) { 
    fprintf(stderr,"USAGE: %s port\n", argv[0]); 
    exit(1);
  } 
  
  // Create the socket that will listen for connections
  int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (listenSocket < 0) {
    error("ERROR opening socket");
  }

 // Set up the address struct for the server socket
  setupAddressStruct(&serverAddress, atoi(argv[1]));

  // Associate the socket to the port
  if (bind(listenSocket, 
          (struct sockaddr *)&serverAddress, 
          sizeof(serverAddress)) < 0){
    error("ERROR on binding");
  }

  // Start listening for connetions. Allow up to 5 connections to queue up
  listen(listenSocket, 5); 
  
  // Accept a connection, blocking if one is not available until one connects
  while(1){
    // Accept the connection request which creates a connection socket
    connectionSocket = accept(listenSocket, 
                (struct sockaddr *)&clientAddress, 
                &sizeOfClientInfo); 
    if (connectionSocket < 0){
      error("ERROR on accept");
    }

    else{
      int childStatus;
      pid_t childPid = fork();

      if(childPid == -1){
        perror("fork() failed!");
        fflush(stdout);
        exit(1);
      } 
      
      // Child process-------------------------------------------------------------------------
      else if(childPid == 0){

      close(listenSocket);

      char buffer[256];
      int count = 0;

      while(count < 10){
          printf("SERVER: Connected to client running at host %d port %d\n", 
                                ntohs(clientAddress.sin_addr.s_addr),
                                ntohs(clientAddress.sin_port));



          // Get the message from the client and display it
          memset(buffer, '\0', 256);
          // Read the client's message from the socket
          charsRead = recv(connectionSocket, buffer, 255, 0); 
          if (charsRead < 0){
            error("ERROR reading from socket");
          }
          printf("SERVER: I received this from the client: \"%s\"\n", buffer);
          sleep(5);
          // Send a Success message back to the client
          charsRead = send(connectionSocket, 
                          "tet", 4, 0); 
          if (charsRead < 0){
            error("ERROR writing to socket");
          }
          count += 1;
          printf("close the loop");
        }
        exit(0);
      }
      
      // Parent process-------------------------------------------------------------------------
      else{
        childPid = waitpid(childPid, &childStatus, WNOHANG);
        printf("Parent process ended");
        // Close the connection socket for this client
        close(connectionSocket);
      }
    }
  }
  // Close the listening socket
  close(listenSocket); 
  return 0;
}
