#include<stdlib.h>
#include<stdio.h>

int main(int argc, char** argv)
{
  int i =1 ;
  for(i =1; i<argc ;i++)
  {
    printf("arg %d:%s \n", i, argv[i]);
  }
  printf("*********I'm done!****************\n");
  return 0;
}
