#include<stdlib.h>
#include<stdio.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include "libnetfiles.h"
// int netopen(const char *pathname, int flags)
// {
// int file = open(pathname, flags);
// if(file == -1)
// {
// perror("There was a problem opening this file");
// return -1;
// }
// printf("netopen was a success!\n");
// close(file);
// return 0;
// }
/*
netserverinit checks if server_name exists
returns 0 on suceess
*/

int netserverinit(const char *server_name, const char* port, struct addrinfo** socket_info, struct addrinfo* hints)
{
  memset(hints, 0, sizeof(hints));
  hints->ai_family = AF_INET;
  hints->ai_socktype  = SOCK_STREAM;
  int error_code = getaddrinfo(server_name, port, hints, socket_info );
  if(error_code!= 0)
  {
      freeaddrinfo(socket_info);
      return HOST_NOT_FOUND;
  }
  return socket(socket_info->ai_family, socket_info->ai_socktype, socket_info->ai_protocol);
}
