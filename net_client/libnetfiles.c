#include<stdlib.h>
#include<stdio.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <netdb.h>
#include "libnetfiles.h"
#include <stdlib.h>
#include <ctype.h>
#include<stdio.h>
#include<unistd.h>
#include <string.h>
#include <errno.h>
int server_socket = 0;  //all net functions will use this socket
int writer(int file_descriptor, char* buffer, int byte_goal, int write_rate)
{
  int total_bytes_written  = 0;
  int bytes_written = 0;
  while(total_bytes_written<byte_goal)
  {
    bytes_written = write(file_descriptor, buffer + total_bytes_written , write_rate);
    if(bytes_written == -1)
    {
      perror("write");
      return -1;
    }
    printf("writer running on %d\n", getpid());
    total_bytes_written += bytes_written;
  }
  return total_bytes_written;
}
int reader(int file_descriptor ,char* buffer, int byte_goal, int read_rate)
{
  int bytes_read  =0;
  int total_bytes_read = 0;
  while(total_bytes_read<byte_goal)
  {
    bytes_read = read(file_descriptor,buffer + total_bytes_read , read_rate);
    if(bytes_read == -1)
    {
      perror("reader:");
      return -1;
    }
    printf("reader running on %d\n", getpid());
    total_bytes_read += bytes_read;
    //printf("total_bytes_read on client side: %d\n", total_bytes_read);
  }
  return total_bytes_read;
}

int reader_delimeter(int file_descriptor ,char* buffer, char* delim)
{
char char_read  = 0;
int byte_read = 0;
int total_bytes_read = 0;
while(char_read!=delim[0])
{
  byte_read =    read(file_descriptor, buffer + total_bytes_read, 1);
  if(byte_read ==  -1)
  {
    perror("reader_delimeter");
    return -1;
  }
  printf("reader_delimeter running on %d\n", getpid());
  char_read =  *(buffer + total_bytes_read);
  //printf("\ncharacter read: %c\n", char_read);
  total_bytes_read += byte_read;
}
//printf("\nbytes returned: %d\n", total_bytes_read);
return total_bytes_read;
}
char* get_command(char* command)
{
  if(strcmp(command, CAT_COMMAND) == 0)
  {
    return CAT_NET;
  }
  else if(strcmp(command ,DOWNLOAD_COMMAND) == 0)
  {
    return DOWNLOAD;
  }
  else if(strcmp(command, UPLOAD_COMMAND))
  {
      return UPLOAD;
  }
  return NULL;
}
int send_request(char* command, char* filepath)
{
char message[MESSAGE_STRING_SIZE + strlen(filepath)];
char message_prep_data[MESSAGE_STRING_SIZE];
char pathname_size_buf[MESSAGE_STRING_SIZE];
//printf("sending request....\n");

//printf("sending this message to server: %s\n", message );
if(strcmp(command, CAT_NET) == 0)
{
  sprintf(pathname_size_buf, "%d", strlen(filepath));
  message_prep(message_prep_data, strlen(command) + strlen(filepath) + strlen(pathname_size_buf) + 3);
  strcpy(message, message_prep_data);
  strcat(message, DELIMETER);
  strcat(message, command);
  strcat(message, DELIMETER);
  strcat(message, pathname_size_buf);
  strcat(message, DELIMETER);
  strcat(message, filepath);
  printf("message being sent to server: %s\n", message );
  if( writer(server_socket, message ,strlen(message), strlen(message)) == -1)
  {
    printf("something crashed on line %d\n", __LINE__);
    return -1;
  }
  int file_size = get_sizeof_server_file(filepath);
  if(file_size == -1)
  {
    printf("something crashed on line %d\n", __LINE__);
    return -1;
  }
  int file_descriptor =  netopen(filepath, O_RDONLY);
  if(file_descriptor == -1)
  {
    printf("something crashed on line %d\n", __LINE__);
    return -1;
  }
  char buffer[BUFFER_SIZE];
  //printf("file_size: %d\n", file_size);
    int total_bytes_read = 0;
    int bytes_read = 0;
    while(total_bytes_read<file_size)
    {
      if((file_size -  total_bytes_read)<= BUFFER_SIZE)
      {
        bytes_read  = netread(file_descriptor, buffer,(file_size -  total_bytes_read));
      }
      else
      {
        bytes_read  = netread(file_descriptor , buffer,BUFFER_SIZE);
      }
    if(bytes_read  == -1)
    {
      //tell the client I screwed up
      printf("something crashed on line %d\n", __LINE__);
      return -1;
    }
    total_bytes_read += bytes_read;
    if (writer(STD_OUT, buffer, bytes_read, bytes_read ) == -1)
    {
      printf("something crashed on line %d\n", __LINE__);
      return -1;
    }
}
if( netclose(file_descriptor) == -1)
{
  printf("something crashed on line %d\n", __LINE__);
  return -1;
}
printf("\n%s\n", BYE);
}
else if(strcmp(command, DOWNLOAD) == 0)
{

  char download_path[strlen(filepath) + strlen(DOWNLOAD_PATH) +1];
   strcpy(download_path, DOWNLOAD_PATH);
   strcat(download_path, filepath);

  int download_file = open(download_path, O_CREAT | O_WRONLY);
  if(download_file == -1)
  {
    printf("something crashed on line %d\n", __LINE__);
    return -1;
  }
  if( fchmod(download_file, S_IRWXU | S_IRWXO) == -1)
  {
    printf("something crashed on line %d\n", __LINE__);
    return -1;
  }

  sprintf(pathname_size_buf, "%d", strlen(filepath));
  message_prep(message_prep_data, strlen(command) + strlen(filepath) + strlen(pathname_size_buf) + 3);
  strcpy(message, message_prep_data);
  strcat(message, DELIMETER);
  strcat(message, command);
  strcat(message, DELIMETER);
  strcat(message, pathname_size_buf);
  strcat(message, DELIMETER);
  strcat(message, filepath);
  printf("message for server: %s\n", message);
  if( writer(server_socket, message ,strlen(message), strlen(message)) == -1)
  {
    printf("something crashed on line %d\n", __LINE__);
    return -1;
  }
  printf("returning from writer\n");
  int file_size = get_sizeof_server_file(filepath);
  printf("file size from server: %d\n", file_size);
  if(file_size == -1)
  {
    printf("something crashed on line %d\n", __LINE__);
    return -1;
  }
  int file_descriptor =  netopen(filepath, O_RDONLY);
  if(file_descriptor == -1)
  {
    printf("something crashed on line %d\n", __LINE__);
    return -1;
  }
    char buffer[BUFFER_SIZE];
  //printf("file_size: %d\n", file_size);
    int total_bytes_read = 0;
    int bytes_read = 0;
    while(total_bytes_read<file_size)
    {
      if((file_size -  total_bytes_read)<= BUFFER_SIZE)
      {
        bytes_read  = netread(file_descriptor, buffer,(file_size -  total_bytes_read));
      }
      else
      {
        bytes_read  = netread(file_descriptor , buffer,BUFFER_SIZE);
      }
    if(bytes_read  == -1)
    {
      //tell the client I screwed up
      printf("\nnetread_server");
      return -1;
    }
    total_bytes_read += bytes_read;
    if( writer(download_file, buffer, bytes_read, bytes_read ) == -1)
    {
      printf("something crashed on line %d\n", __LINE__);
      return -1;
    }

  }
  if( netclose(file_descriptor) == -1)
  {
    printf("something crashed on line %d\n", __LINE__);
    return -1;
  }
  if(close(download_file) == -1)
  {
    printf("something crashed on line %d\n", __LINE__);
    return -1;
  }
    printf("\n%s\n", BYE);
}
else
{
  return -1;
}
return 0;

}

int netclose(int file_descriptor)
{
  char message_prep_data[MESSAGE_STRING_SIZE];
  char file_descriptor_data[MESSAGE_STRING_SIZE];
  sprintf(file_descriptor_data, "%d", file_descriptor);
  message_prep(message_prep_data, strlen(file_descriptor_data) + 4);
  char message[strlen(file_descriptor_data) + strlen(message_prep_data) + 5];
  strcpy(message, message_prep_data);
  strcat(message, DELIMETER);
  strcat(message, CLOSE);
  strcat(message, DELIMETER);
  strcat(message, file_descriptor_data);
  strcat(message, DELIMETER);
  if( writer(server_socket, message, strlen(message), strlen(message)) == -1)
  {
    printf("something crashed on line %d\n", __LINE__);
    return -1;
  }
  if(interpret_message() ==-1)
  {
    printf("something crashed on line %d\n", __LINE__);
    return -1;
  }
}


int get_io_flags(char* str)
{
  if(strcmp(str, CAT_NET) == 0 )
  {
    //printf("read only flag\n");
      return O_RDONLY;
  }

  else if(strcmp(str, UPLOAD ) == 0)
  {
    //printf("write only flag\n");
      return O_WRONLY;
  }
  else if(strcmp(str, READ_AND_WRITE) == 0)
  {
    //printf("Read and write\n");
    return O_RDWR;
  }
  return -1;
}
int sizeof_message()
{
  char message_size[MESSAGE_STRING_SIZE];
  message_size[reader_delimeter( server_socket,message_size, DELIMETER) - 1] = '\0';  //NOTE: This replaces the delimeter with a null-byte(NOTE the -1 means "replace the last character with a null-byyte")
  return atoi(message_size);
}

void message_prep(char* message, int size)
{
  char message_size[MESSAGE_STRING_SIZE];
  sprintf(message_size, "%d" ,size);
  int Numberofdigits_size = strlen(message_size);
  sprintf(message_size, "%d" ,size + Numberofdigits_size);
  strcpy(message, message_size);
  return;
}
char  status_of_message()
{
  char message_status[MESSAGE_STRING_SIZE];
  message_status[reader_delimeter(server_socket, message_status, DELIMETER)-1] = '\0';
  //printf("comparing %s and %s\n", message_status, MESSAGE_SUCCESS );
  if(strcmp(message_status, MESSAGE_SUCCESS) == 0)
      return MESSAGE_SUCCESS[0];

  return MESSAGE_ERROR[0];
}
int interpret_message()
{
  //printf("intrepreting message on client\n");
int message_size  =  sizeof_message();
//printf("calculatin size of message was successful\n");
char error_code[MESSAGE_STRING_SIZE];
char message_size_data[MESSAGE_STRING_SIZE];
sprintf(message_size_data,"%d",message_size);
char message = status_of_message();
//printf("message code from server %c\n", message);
if(message == MESSAGE_ERROR[0])
{
  //printf("retuning -1 from netopen on client side\n");
  error_code[reader(server_socket, error_code, message_size -(3 + strlen(message_size_data)), message_size -(3 + strlen(message_size_data)))] = '\0'; //data should contain the errno code sent from server after the reader call
  errno = atoi(error_code);
  perror("intrepret message:\n");
  return -1;
}
//printf("returning success from netopen on client side\n");
return message_size - 3  ;
}
int netopen(const char *pathname, int flags)
{
  char flags_string[MESSAGE_STRING_SIZE];
  char pathname_size[MESSAGE_STRING_SIZE];
  sprintf(pathname_size, "%d", strlen(pathname));
  //printf("path given to netopen: %s\n", pathname);
  sprintf(flags_string,"%d",flags);
  char message_data[MESSAGE_STRING_SIZE];
  message_prep(message_data, (strlen(pathname) + strlen(flags_string) + strlen(pathname_size) + 6) );
  char* message = malloc(strlen(pathname) + strlen(flags_string)  + strlen(message_data) + strlen(pathname_size) + 7);
  strcpy(message, message_data);
  strcat(message, DELIMETER);
  strcat(message, OPEN);
  strcat(message, DELIMETER);
  strcat(message, pathname_size);
  strcat(message, DELIMETER);
  strcat(message, pathname);
  strcat(message, flags_string);
  strcat(message, DELIMETER);
   writer(server_socket, message, strlen(message), strlen(message  ));
  //printf("message sent to server: %s\n", message);
//  printf( "Client wrote %d bytes\n" , ;
    int server_response_size =  interpret_message();
    if(server_response_size == -1)
    {
      perror("netopen");
      return -1;
    }
    //printf("server_response_size: %d\n", server_response_size);
    char server_file_decriptor[MESSAGE_STRING_SIZE];
    server_file_decriptor[reader_delimeter(server_socket, server_file_decriptor, DELIMETER) -1] = '\0';
    //printf("data returned from server: %s\n", server_file_decriptor );
    return atoi(server_file_decriptor);
}
int get_sizeof_server_file(char* file_path)
{
  char message_prep_data[MESSAGE_STRING_SIZE];
  char file_size_data[MESSAGE_STRING_SIZE];
  char filepath_size[MESSAGE_STRING_SIZE];
  sprintf(filepath_size,  "%d", strlen(file_path));
  message_prep(message_prep_data,  strlen(file_path)+ strlen(filepath_size) + strlen(SIZE_OF_FILE) + 3);
  char message[strlen(message_prep_data) + strlen(file_path) + strlen(filepath_size) + strlen(SIZE_OF_FILE) + 4];
  strcpy(message, message_prep_data);
  strcat(message, DELIMETER);
  strcat(message, SIZE_OF_FILE);
  strcat(message, DELIMETER);
  strcat(message, filepath_size);
  strcat(message, DELIMETER);
  strcat(message, file_path);
  //printf("sending this message to server: %s\n", message);
  if( writer(server_socket, message ,strlen(message), strlen(message) ) == -1)
  {
    return -1;
  }
  if (interpret_message() ==  -1)
  {
    printf("there was a problem intrepreting the message\n");
    return -1;
  }
  file_size_data[reader_delimeter(server_socket, file_size_data, DELIMETER) - 1] = '\0';
  return atoi(file_size_data);
}
ssize_t netread(int server_fildes, void *buf, size_t byte_goal)
{
  char* buf_ptr  = (char*)buf;
  char server_fildes_data[MESSAGE_STRING_SIZE];
  char server_byte_goal_data[MESSAGE_STRING_SIZE];
  sprintf(server_fildes_data, "%d",server_fildes );
  sprintf(server_byte_goal_data, "%d", byte_goal);
  char message_prep_data[MESSAGE_STRING_SIZE];
  message_prep(message_prep_data, strlen(server_fildes_data) + strlen(server_byte_goal_data) + 5);
  char message[strlen(message_prep_data) + strlen(server_fildes_data) + strlen(server_byte_goal_data) +6];
  strcpy(message, message_prep_data);
  strcat(message, DELIMETER);
  strcat(message, READ);
  strcat(message, DELIMETER);
  strcat(message, server_fildes_data);
  strcat(message, DELIMETER);
  strcat(message, server_byte_goal_data);
  strcat(message, DELIMETER);
  //printf("sending this to server: %s\n", message);
  if(writer(server_socket, message,strlen(message), strlen(message)) == -1)
  {
    perror("netread");
    return -1;
  }
  //printf("netread client side\n");
  int sizeof_server_file  = interpret_message();
  if(sizeof_server_file  == -1)
  {
    perror("netopen on client: %d\n");
    return -1;
  }
  char sizeof_server_file_data[MESSAGE_STRING_SIZE];
  sizeof_server_file_data[reader_delimeter(server_socket, sizeof_server_file_data, DELIMETER) - 1] = '\0';
  sizeof_server_file = atoi(sizeof_server_file_data);
  //printf("size of server file: %d\n", sizeof_server_file);
  reader(server_socket ,buf_ptr, sizeof_server_file, sizeof_server_file );
   //buf_ptr[sizeof_server_file] = '\0';
   //printf (" content of buffer: %s\n", buf_ptr);
  return sizeof_server_file;
}
ssize_t netwrite(int server_fildes, const void *buf, size_t byte_goal)
{
  char* buf_ptr  = (char*)buf;
  char server_fildes_data[MESSAGE_STRING_SIZE];
  char server_byte_goal_data[MESSAGE_STRING_SIZE];
  sprintf(server_fildes_data, "%d",server_fildes );
  sprintf(server_byte_goal_data, "%d", byte_goal);
  char message_prep_data[MESSAGE_STRING_SIZE];
  message_prep(message_prep_data, strlen(server_fildes_data) + strlen(server_byte_goal_data) + 5 + byte_goal);
  char message[strlen(message_prep_data) + strlen(server_fildes_data) + strlen(server_byte_goal_data) +6 + byte_goal];
  strcpy(message, message_prep_data);
  strcat(message, DELIMETER);
  strcat(message, WRITE);
  strcat(message, DELIMETER);
  strcat(message, server_fildes_data);
  strcat(message, DELIMETER);
  strcat(message, server_byte_goal_data);
  strcat(message, DELIMETER);
  strcat(message, buf);
  //printf("sending this to server: %s\n", message);
  if(writer(server_socket, message,strlen(message), strlen(message)) == -1)
  {
    perror("netread");
    return -1;
  }
  return 0;
}
int netserverinit(const char *server_name, const char* port, struct addrinfo** socket_info, struct addrinfo* hints)
{
  memset(hints, 0, sizeof(struct addrinfo));
  hints->ai_family = PF_INET;
  hints->ai_socktype  = SOCK_STREAM;
  int error_code = getaddrinfo(server_name, port, hints, socket_info );
  if(error_code!= 0)
  {
      perror("getaddrinfo");
      freeaddrinfo(*socket_info);
      return -1;
  }
  //printf("From netserverinit: %s\n", (*socket_info)->ai_canonname);
  server_socket =  socket((*socket_info)->ai_family, (*socket_info)->ai_socktype, (*socket_info)->ai_protocol);
  if(server_socket==-1)
  {
    perror("socket");
    return -1;
  }
  return 0;
}
