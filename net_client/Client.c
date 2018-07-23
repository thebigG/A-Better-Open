/****************** CLIENT CODE ****************/

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include<stdlib.h>
#include <netdb.h>
#include <sys/uio.h>
#include <unistd.h>
#include<err.h>
#include <errno.h>
#include <fcntl.h>
#include "libnetfiles.h"

int extern server_socket;
int main(int argc, char** argv)
{
  printf("starting up this one: %d\n", getpid());
  if(chdir(WORKING_DIRECTORY) == 0)
  {
      printf("changing working directory was a sucess\n");
  }
  else
  {
    perror("Something went wrong");
    return -1;
  }
  if(argc!=5 )
  {
  fprintf(stderr,"No enough arguments to connect to server\n");
  printf("%d\n", argc);
  return -1;
  }
  char* command = malloc((sizeof(char) * strlen(argv[1])) + 1 );
  char* path_name = malloc((sizeof(char) * strlen(argv[2]) ) + 1);
  char* ServerName  = malloc((sizeof(char)* strlen(argv[3])) + 1);
  char* port = malloc((sizeof(char)* strlen(argv[4])) + 1);
  //char *buffer =  malloc(sizeof(char) * 256);
  struct addrinfo hints;
  struct addrinfo*socket_info;
  strcpy(command, argv[1]);
  strcpy(path_name, argv[2]);
  strcpy(ServerName, argv[3]);
  strcpy(port, argv[4]);
  netserverinit(ServerName, port,&socket_info, &hints );
  if(server_socket == -1)
  {
   perror("netserverinit");
   return -1;
  }
  struct addrinfo* socket_info_ptr = socket_info;

     if(connect(server_socket, (socket_info_ptr)->ai_addr,(socklen_t) socket_info_ptr->ai_addrlen) == -1)
     {
         close(server_socket);
          perror("connect");
          return -1;
     }

    if(send_request( get_command(command), path_name) == -1)
    {
      perror("request could be sent\n");
      printf("request could be sent\n");
      return -1;
    }
    printf("This seems fine\n");
    if(close(server_socket) ==-1)
    {
      perror("problem closing socket");
      return -1;
    }
    printf("Succesful for %d \n", getpid());
  return 0;
}
