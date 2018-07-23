#include <unistd.h>
#include <stdio.h>
#include<string.h>
#include<stdlib.h>
int main (int argc, char** argv)
{
   int i =0;
  char* client_data[6] = {NULL};
  for(i =1;i<argc;i++)
  {
    client_data[i-1] = argv[i];

    printf("string in list: %s\n", client_data[i-1]);
  }
  char** current = client_data;
   while(*current)
  {
    printf("in loop string: %s\n", *current);
    current++;
  }
  fork();
  fork();
  printf("Running:%s\n", client_data[0] );
  execv("./client", client_data );
  perror("execv");
}
