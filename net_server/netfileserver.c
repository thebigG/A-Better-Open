/****************** SERVER CODE ****************/
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include "libnetfiles_server.h"
extern int server_socket;
int main(int argc, char** argv)
{
  if(argc != 2 )
  {
    printf("Unable to compute arguments:\n"
    "USAGE: ./netfileserver PORT(between 8192 and 65536)\n");
    return -1;
  }
  if(chdir(WORKING_DIRECTORY) == 0)
  {
      printf("changing working directory was a sucess\n");
  }
  else
  {
    perror("Something went wrong");
  }
  int client_socket = 0;
  char buffer[1024];
  struct addrinfo hints ;
  struct addrinfo *socket_info;
  char* port  = malloc(6 * sizeof(char));
  strcpy(port, argv[1]);
   if(( netserverinit_server(port, &socket_info, &hints)) == -1 )
   {
      return -1;
   }
//NOTE: Maybe it'll be a good idea to paramatize the backlog(second argument) of listen(?)
  if(listen(server_socket,5)==0)
  printf("Listening\n");
  else
  printf("Error\n");
  //split("12|./filepath|third|", DELIMETER);
  while(1){
  client_socket = accept(server_socket, NULL, NULL);
  //create thread, probably
  if(client_socket==-1)
  perror("Accept");
  pthread_t thread;

   //handle_request(client_socket);

  if(pthread_create(&thread, NULL, handle_request, &client_socket ) == -1)
  {
    perror("pthread_create");
    return -1;
  }
  else
  {
    printf("firing off thread for: %d\n", client_socket);
  }

  if(  pthread_detach(thread) == -1)
  {
  perror("pthread_detach");
  }
   printf("deatched the thread");
}
  return 0;
}
