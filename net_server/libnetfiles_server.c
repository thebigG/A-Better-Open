#include<stdlib.h>
#include<stdio.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <string.h>
#include <netdb.h>
#include <stdlib.h>
#include <ctype.h>
#include<stdio.h>
#include <string.h>
#include <errno.h>
#include<unistd.h>
#include<pthread.h>
#include "libnetfiles_server.h"
int server_socket = 0;    //Not sure why this is global, but it works as it is; we'll leave like this, for now.
pthread_mutex_t lock;
void message_prep_server(char* message, int size, const char* status)
{

  char message_size[MESSAGE_STRING_SIZE];
  sprintf(message_size, "%d" ,size);
  int Numberofdigits_size = strlen(message_size);
  sprintf(message_size, "%d" ,size + Numberofdigits_size + strlen(status));
  //printf("messsage size form server: %s\n", message_size );
  strcpy(message, message_size);
  strcat(message, DELIMETER);
  strcat(message, status);
  return;
}
char* getSubStr(const char* src, int start, int end)
{
  char* Sub = malloc((sizeof(char) * (end - start)) + 1);
  strncpy(Sub, (src+ start), end - start);
  Sub[end - start] = '\0';
  printf("returning  substr: %s\n", Sub);
  printf("length of substr: %d\n",strlen(Sub) );
  return Sub;
}

int count_tokens(char* str, char* delimeter)
{
char* current_token = str;
char* last_delimeter = NULL;
char* current_delimeter_char = delimeter;
int current_token_count = 0;
int delimeter_len = strlen(delimeter);
// printf("Is this function running");

while((*current_token) != '\0')
{
  // printf("Is this loop running");
  if(*current_token == *delimeter)
    {
      if(delimeter_len>1 && ( (*(current_token + delimeter_len)) !='\0'))
      {
        current_delimeter_char = delimeter;
        while( *current_delimeter_char!= '\0')
        {
          // printf("Doing more\n");
          if (*current_token != *current_delimeter_char)
          {
            continue;
          }
          current_token++;
          current_delimeter_char++;
      }
      last_delimeter = current_token;
      current_token_count++;
      }
      else
      {
        last_delimeter = current_token;
        current_token_count++;
      }

    }
    current_token++;

}

if((*(last_delimeter + 1)) != '\0')
{
  current_token_count++;
  printf("Special case on count: %d\n", current_token_count );
}
return current_token_count;
}

/*
Splits a string
*/
char** split(char* str, char* delimeter)
{
int token_count  = (count_tokens(str, delimeter)) + 1;
printf("Count went fine: %d\n", token_count);
char** str_list = NULL;
int str_len = 0;
char** str_list_current  = NULL;
int delimeter_len = 0;
char* current_char  = NULL;
int offset = 0;
int current_char_index = 0;
char* last_delimeter = NULL;
int end  = 0;
if(token_count > 0 )
{
 str_list = malloc((sizeof(char*)) * token_count );
 str_len = strlen(str);
 str_list_current  = str_list;
 delimeter_len = strlen(delimeter);
 current_char  = str;
 offset = 0;
 current_char_index = 0;
 last_delimeter = NULL;
 end  = 0;
while(*current_char != '\0')
{
  if(strncmp(current_char, delimeter, delimeter_len) == 0)
  {
    current_char+= delimeter_len;
    last_delimeter = current_char;
    printf("normal substr call\n");
    *str_list_current = getSubStr(str, current_char_index- offset, current_char_index);
    current_char_index += delimeter_len;
    str_list_current++;
    offset = 0;
    continue;
  }
  offset++;
  current_char_index++;
  current_char++;
}

// printf("Odd\n");
  if(token_count>2)
  {
  if(*(last_delimeter) !='\0')
  {
    printf("Does this happen?");
    *str_list_current =  getSubStr(str, current_char_index- offset, current_char_index  );
    printf("getSub went fine\n");
  }
}
}
str_list[token_count-1] = NULL;
printf("Returning from split\n");

return str_list;
}
int writer(int file_descriptor, char* buffer, int byte_goal, int write_rate)
{
  int total_bytes_written  = 0;
  int bytes_written = 0;
  while(total_bytes_written<byte_goal)
  {
    bytes_written = write(file_descriptor, buffer + total_bytes_written , write_rate);
    if(bytes_written == -1)
    {
      perror("writer");
      return -1;
    }
    printf(" running writer for %d on file_descriptor: %d\n " ,pthread_self(), file_descriptor);
    total_bytes_written += bytes_written;
  }
  return total_bytes_written;
}
int netopen_server(const char* file_path, int client_socket, int flags )
{

    int file_descriptor = open(file_path, flags);
    char file_descriptor_string[32];
    char message_header[MESSAGE_STRING_SIZE];
    printf("path given to netopen: %s\n", file_path);
    printf("running netserver...open\n");
    if(file_descriptor == -1)
    {
      sprintf(file_descriptor_string, "%d", errno);
      message_prep_server(message_header ,strlen(file_descriptor_string) + 1, MESSAGE_ERROR);
      char message[strlen(message_header) + strlen(file_descriptor_string)+ strlen(MESSAGE_ERROR) + 2];
      strcpy(message, message_header);
      strcat(message, DELIMETER);
      strcat(message, file_descriptor_string);
      printf("errno value: %d\n", errno);
      printf("sendding erro message to client: %s\n", message );
      writer(client_socket,message, strlen(message), 1 );

      return -1;
    }
    file_descriptor *= -1;
    sprintf(file_descriptor_string, "%d", file_descriptor);
    message_prep_server(message_header, strlen(file_descriptor_string) + 1, MESSAGE_SUCCESS );
    char message[strlen(message_header) + strlen(file_descriptor_string) + 2];
    strcpy(message, message_header);
    strcat(message, DELIMETER);
    strcat(message, file_descriptor_string);
    strcat(message, DELIMETER);
    writer(client_socket,message, strlen(message), strlen(message));
    printf("wrote  this:%s\nfrom client\n", message);
    printf("returning from netopen on server side this file_descriptor: %d\n", file_descriptor );
    return (file_descriptor * -1 );
}
//NOTE: if netread_server is being used with netread from client, byte_goal SHALL NOT exceed BUFFER_SIZE.
//If byte_goal DOES exceed BUFFER_SIZE, then make sure the appropiate steps are being take on the client size.
int netread_server(int fd_client, int fd_flie, int byte_goal)
{
  char buf[BUFFER_SIZE];
  int total_bytes_read = 0;
  int message_size = 0;
  int bytes_read = 0;
  printf("netread_server running\n");
  char message_size_data[MESSAGE_STRING_SIZE];
  char fd_file[MESSAGE_STRING_SIZE];
  char message[BUFFER_SIZE + MESSAGE_STRING_SIZE +1];
  char bytes_read_data[MESSAGE_STRING_SIZE];
  while(total_bytes_read<byte_goal)
  {
    if((byte_goal -  total_bytes_read)<= BUFFER_SIZE)
    {
      bytes_read  = reader((fd_flie ), buf,(byte_goal -  total_bytes_read), (byte_goal -  total_bytes_read));
    }
    else
    {
      bytes_read  = reader((fd_flie ), buf,BUFFER_SIZE, BUFFER_SIZE);
    }
  if(bytes_read  == -1)
  {
    //tell the client I screwed up
    perror("\nnetread_server");
    return -1;
  }
  total_bytes_read += bytes_read;
  sprintf(bytes_read_data, "%d", bytes_read);
  message_prep_server(message_size_data, bytes_read +2 + strlen(bytes_read_data), MESSAGE_SUCCESS);
  strcpy(message, message_size_data);
  strcat(message, DELIMETER);
  strcat(message, bytes_read_data );
  strcat(message, DELIMETER);
  message_size = bytes_read + strlen(message) +1;
  memcpy(message + strlen(message), buf,bytes_read );
  //printf("sending this to client: %s\n", message );
  if (writer(fd_client,message, message_size, message_size) == -1)
  {
    printf("problem with writer on server side\n");
    perror("netread_server");
  }
 }
 //reader(fd_client, buf, 1, 1 );
 printf("returning from netread on server side\n");
return total_bytes_read;
}
int netwrite_server(int fd_client, int fd_file, int byte_goal)
{
  char buf[BUFFER_SIZE];
  int total_bytes_read = 0;
  int message_size = 0;
  char message_size_data[MESSAGE_STRING_SIZE];
  char message[BUFFER_SIZE + MESSAGE_STRING_SIZE +1];
  while(total_bytes_read<byte_goal)
  {
    if((byte_goal -  total_bytes_read)<= BUFFER_SIZE)
    {
      message_size  = reader((fd_client ), buf,(byte_goal -  total_bytes_read), (byte_goal -  total_bytes_read));
      if (message_size == -1)
      {
        //atempt to tell the client I screwed up(maybe?)
      }
        message_size = writer(fd_file, buf,(byte_goal -  total_bytes_read), (byte_goal -  total_bytes_read));
        if (message_size == -1)
        {
          //atempt to tell the client I screwed up(maybe?)
        }
    }
    else
    {
      message_size = reader((fd_client ), buf,BUFFER_SIZE,BUFFER_SIZE);
      if (message_size == -1)
      {
        //atempt to tell the client I screwed up(maybe?)
      }
      message_size  = writer((fd_file ), buf,BUFFER_SIZE, BUFFER_SIZE);
      if (message_size == -1)
      {
        //atempt to tell the client I screwed up(maybe?)
      }
    }
  if(message_size  == -1)
  {
    //tell the client I screwed up
    perror("\nnetread_server");
    return -1;
  }
  total_bytes_read += message_size;
  message_prep_server(message_size_data, message_size + 1, MESSAGE_SUCCESS);
  strcpy(message, message_size_data);
  strcat(message, DELIMETER);
  printf("sending this to client: %s\n", message );
  writer(fd_client,message, strlen(message), strlen(message));
}
return 0;
}

int netserverinit_server( const char* port, struct addrinfo** socket_info, struct addrinfo* hints)
{
  memset(hints, 0, sizeof(struct addrinfo));
  hints->ai_family = PF_INET;
  hints->ai_flags = AI_PASSIVE;
  hints->ai_socktype  = SOCK_STREAM;
  int error_code = getaddrinfo(NULL, port, hints, socket_info );
  if(error_code!= 0)
  {
      perror("getaddrinfo");
      freeaddrinfo(*socket_info);
      return -1;
  }
  server_socket  = socket((*socket_info)->ai_family, (*socket_info)->ai_socktype, (*socket_info)->ai_protocol);;
  if(server_socket == -1)
  {
    perror("socket");
    return -1;
  }
  if(bind(server_socket, (*socket_info)->ai_addr, (*socket_info)->ai_addrlen) == -1)
  {
    perror("bind");
    return -1;
  }
  return 0;

}
/*
Wrapper function for read
It esnures that all bytes are read
*/
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
    printf("running  reader for %d  on file: %d\n" ,pthread_self(), file_descriptor);
    total_bytes_read += bytes_read;
    //printf("total_bytes_read on server_side: %d\n", total_bytes_read);
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
  byte_read =  read(file_descriptor, buffer + total_bytes_read, 1);
  if(byte_read ==  -1)
  {
    perror("reader_delimeter");
    return -1;
  }
  char_read =  *(buffer + total_bytes_read);
  //printf("character read: %c\n", char_read);
  printf("running reader_delimeter for %d on file: %d\n", pthread_self(), file_descriptor);
  total_bytes_read += byte_read;
}
printf("bytes returned: %d\n", total_bytes_read);
return total_bytes_read;
}

int sizeof_message(int fd_client)
{
  char message_size[MESSAGE_STRING_SIZE];
  message_size[reader_delimeter( fd_client,message_size, DELIMETER) -1] = '\0';
  return atoi(message_size);
}
 int get_filesize(char* file_path)
 {
 struct stat data;
 if (stat(file_path, &data ) == -1)
 {
   return -1;
 }
 printf("size on server side: %d\n", data.st_size);
 return data.st_size;
 }
 /*
 eval_command SHALL be used on a test-once logical structure
 DO NOT use eval _command(with the same command) in a more-than-one logical structre(switch, else-if)
In other words, it should NOT be used more than once on the same command.
If doing so, beware that the behavior for using this function on the same command is UNDEFINED.
 */
 int eval_command(char* command, int fd_client)
 {
   char* client_command = read_message(fd_client);
   printf("command passed to function: %s\n", command);
   if(strcmp(client_command, command) == 0)
   {
     free(client_command);
     printf("returning from command\n");
     return 0;
   }
   printf("returning from command with -1\n");
   return -1;
 }
 void handle_request(void* fd_client_data)
 {
      int fd_client  = *((int*)(fd_client_data));
      char* command  = read_message(fd_client);
      if(strcmp(CAT_NET, command) == 0)
      {
         pthread_mutex_lock(&lock);
        printf("this command has been evaluated in cat_net: \n");
        int file_descriptor  =0;
        int filepath_size = 0;
        int flags =0 ;
        char original_path_size_data[BUFFER_SIZE];
          original_path_size_data[reader_delimeter(fd_client, original_path_size_data, DELIMETER ) - 1] = '\0';
        int original_path_size = atoi(original_path_size_data);
        char original_path[original_path_size + 1];
        original_path[reader(fd_client, original_path, original_path_size, original_path_size )] = '\0';
        char flags_data[MESSAGE_STRING_SIZE];
        char filepath_size_buf[MESSAGE_STRING_SIZE];
        if(eval_command(SIZE_OF_FILE, fd_client) != 0)
        {
          printf("invalid command\n");
          char  message_header[MESSAGE_STRING_SIZE];
          char error_code_string[MESSAGE_STRING_SIZE];
          sprintf(error_code_string, "%d", errno);
          message_prep_server(message_header ,strlen(error_code_string) + 1, MESSAGE_ERROR);
          char message[strlen(message_header) + strlen(error_code_string)+ strlen(MESSAGE_ERROR) + 2];
          strcpy(message, message_header);
          strcat(message, DELIMETER);
          strcat(message, error_code_string);
          printf("errno value: %d\n", errno);
          printf("sendding erro message to client: %s\n", message );
          writer(fd_client,message, strlen(message), 1 );
          close(fd_client);
          pthread_exit(NULL);

          //attempt to let the client know about this error
        }
        filepath_size_buf[reader_delimeter(fd_client, filepath_size_buf, DELIMETER) - 1] = '\0';
        filepath_size = atoi(filepath_size_buf);
        char path[filepath_size + 1];
        path[reader(fd_client, path,filepath_size, filepath_size )] = '\0';
        char filesize_data[MESSAGE_STRING_SIZE];
        //lock mutex here
        int file_size = get_filesize(path );

        if(file_size ==-1)
        {
          char  message_header[MESSAGE_STRING_SIZE];
          char error_code_string[MESSAGE_STRING_SIZE];
          sprintf(error_code_string, "%d", errno);
          message_prep_server(message_header ,strlen(error_code_string) + 1, MESSAGE_ERROR);
          char message[strlen(message_header) + strlen(error_code_string)+ strlen(MESSAGE_ERROR) + 2];
          strcpy(message, message_header);
          strcat(message, DELIMETER);
          strcat(message, error_code_string);
          printf("errno value: %d\n", errno);
          printf("sendding erro message to client: %s\n", message );
          writer(fd_client,message, strlen(message), 1 );
          printf("invalid command\n");
          printf("bad file descriptor for size file function\n");
          close(fd_client);
          pthread_exit(NULL);


        }
        sprintf(filesize_data  ,"%d" ,file_size );
        char message_prep_data[MESSAGE_STRING_SIZE];
        message_prep_server(message_prep_data, strlen(filesize_data) + 2, MESSAGE_SUCCESS);
        char message[strlen(message_prep_data) + strlen(filesize_data) + 3];
        strcpy(message, message_prep_data);
        strcat(message, DELIMETER);
        strcat(message, filesize_data);
        strcat(message, DELIMETER);
        printf("writing this to client: %s\n", message );
        writer(fd_client, message, strlen(message), strlen(message));
        if(eval_command(OPEN, fd_client) != 0)
        {
          printf("invalid command\n");
          char error_code_string[MESSAGE_STRING_SIZE];
          char  message_header[MESSAGE_STRING_SIZE];
          sprintf(error_code_string, "%d", errno);
          message_prep_server(message_header ,strlen(error_code_string) + 1, MESSAGE_ERROR);
          char message[strlen(message_header) + strlen(error_code_string)+ strlen(MESSAGE_ERROR) + 2];
          strcpy(message, message_header);
          strcat(message, DELIMETER);
          strcat(message, error_code_string);
          printf("errno value: %d\n", errno);
          printf("sendding erro message to client: %s\n", message );
          writer(fd_client,message, strlen(message), 1 );
          printf("invalid command\n");
          close(fd_client);
          pthread_exit(NULL);
        }
        filepath_size_buf[reader_delimeter(fd_client, filepath_size_buf, DELIMETER) - 1] = '\0';
        filepath_size = atoi(filepath_size_buf);
        char open_pathname[filepath_size];
        open_pathname[reader(fd_client, open_pathname ,filepath_size, filepath_size )] = '\0';
        flags_data[reader_delimeter(fd_client, flags_data, DELIMETER) - 1] = '\0';

        printf("calling net open...\n");
        file_descriptor  = netopen_server(open_pathname, fd_client ,atoi(flags_data) );
        if(file_descriptor ==-1)
        {
          printf("bad file descriptor");
          printf("invalid command\n");
          char  message_header[MESSAGE_STRING_SIZE];
          char error_code_string[MESSAGE_STRING_SIZE];
          sprintf(error_code_string, "%d", errno);
          message_prep_server(message_header ,strlen(error_code_string) + 1, MESSAGE_ERROR);
          char message[strlen(message_header) + strlen(error_code_string)+ strlen(MESSAGE_ERROR) + 2];
          strcpy(message, message_header);
          strcat(message, DELIMETER);
          strcat(message, error_code_string);
          printf("errno value: %d\n", errno);
          printf("sendding erro message to client: %s\n", message );
          writer(fd_client,message, strlen(message), 1 );
          close(fd_client);
          pthread_exit(NULL);
        }
       printf("file descriptor on server side: %d\n", file_descriptor);




       char read_file_descriptor_data[MESSAGE_STRING_SIZE];
       char bytegoal_data[MESSAGE_STRING_SIZE];
       int byte_goal = 0;
       int read_file_descriptor = 0 ;
       int bytes_read  = 0;
       int total_bytes_read = 0;
      while(total_bytes_read<file_size)
       {
         if(eval_command(READ, fd_client) != 0)
         {
           printf("invalid command\n");
           char  message_header[MESSAGE_STRING_SIZE];
           char error_code_string[MESSAGE_STRING_SIZE];
           sprintf(error_code_string, "%d", errno);
           message_prep_server(message_header ,strlen(error_code_string) + 1, MESSAGE_ERROR);
           char message[strlen(message_header) + strlen(error_code_string)+ strlen(MESSAGE_ERROR) + 2];
           strcpy(message, message_header);
           strcat(message, DELIMETER);
           strcat(message, error_code_string);
           printf("errno value: %d\n", errno);
           printf("sendding erro message to client: %s\n", message );
           writer(fd_client,message, strlen(message), 1 );
           close(fd_client);
           pthread_exit(NULL);
         }
         read_file_descriptor_data[reader_delimeter(fd_client, read_file_descriptor_data, DELIMETER) - 1] = '\0';
         bytegoal_data[reader_delimeter(fd_client, bytegoal_data, DELIMETER) - 1] = '\0';
         byte_goal = atoi(bytegoal_data);
         read_file_descriptor = atoi(read_file_descriptor_data);
         bytes_read =  netread_server(fd_client, read_file_descriptor * -1 , byte_goal);

         if(bytes_read == -1)
         {
           printf("invalid command\n");
           char  message_header[MESSAGE_STRING_SIZE];
           char error_code_string[MESSAGE_STRING_SIZE];
           sprintf(error_code_string, "%d", errno);
           message_prep_server(message_header , strlen(error_code_string) + 1, MESSAGE_ERROR);
           char message[strlen(message_header) + strlen(error_code_string)+ strlen(MESSAGE_ERROR) + 2];
           strcpy(message, message_header);
           strcat(message, DELIMETER);
           strcat(message, error_code_string);
           printf("errno value: %d\n", errno);
           printf("sendding erro message to client: %s\n", message );
           writer(fd_client,message, strlen(message), 1 );
           close(fd_client);
           pthread_exit(NULL);
         }
         total_bytes_read += bytes_read;
         printf("total bytes read: %d and  byte_goal: %d\n", total_bytes_read, byte_goal);
       }
       if(eval_command(CLOSE, fd_client) != 0)
       {
         printf("invalid command\n");
         char  message_header[MESSAGE_STRING_SIZE];

         char error_code_string[MESSAGE_STRING_SIZE];
         sprintf(error_code_string, "%d", errno);
         message_prep_server(message_header ,strlen(error_code_string) + 1, MESSAGE_ERROR);
         char message[strlen(message_header) + strlen(error_code_string)+ strlen(MESSAGE_ERROR) + 2];
         strcpy(message, message_header);
         strcat(message, DELIMETER);
         strcat(message, error_code_string);
         printf("errno value: %d\n", errno);
         printf("sendding erro message to client: %s\n", message );
         writer(fd_client,message, strlen(message), 1 );
         printf("invalid command\n");
         pthread_exit(NULL);
       }
        char close_file_descriptor_data[MESSAGE_STRING_SIZE];
        close_file_descriptor_data[reader_delimeter(fd_client, close_file_descriptor_data, DELIMETER) - 1] ='\0';
        int close_file_descriptor = atoi(close_file_descriptor_data);
         if(netclose_server(fd_client , close_file_descriptor * -1) == -1)
         {
           char error_code_string[MESSAGE_STRING_SIZE];
           char  message_header[MESSAGE_STRING_SIZE];
           sprintf(error_code_string, "%d", errno);
           message_prep_server(message_header ,strlen(error_code_string) + 1, MESSAGE_ERROR);
           char message[strlen(message_header) + strlen(error_code_string)+ strlen(MESSAGE_ERROR) + 2];
           strcpy(message, message_header);
           strcat(message, DELIMETER);
           strcat(message, error_code_string);
           printf("errno value: %d\n", errno);
           printf("sendding erro message to client: %s\n", message );
           printf("invalid command\n");
           writer(fd_client,message, strlen(message), 1 );
           close(fd_client);
           pthread_mutex_unlock(&lock);
           pthread_exit(NULL);
         }
         printf("This seems fine\n");
      }


      else if(strcmp(DOWNLOAD, command) == 0)
      {
        printf("this command has been evaluated in dowload: \n");
        int file_descriptor  =0;
        int filepath_size = 0;
        int flags =0 ;
        char original_path_size_data[BUFFER_SIZE];
        original_path_size_data[reader_delimeter(fd_client, original_path_size_data, DELIMETER ) - 1] = '\0';
        int original_path_size = atoi(original_path_size_data);
        char original_path[original_path_size + 1];
        original_path[reader(fd_client, original_path, original_path_size, original_path_size )] = '\0';
        printf("original path: %s\n", original_path);
        char flags_data[MESSAGE_STRING_SIZE];
        char filepath_size_buf[MESSAGE_STRING_SIZE];
        if(eval_command(SIZE_OF_FILE, fd_client) != 0)
        {
          printf("invalid command\n");
          //let the client know about this error
          return;
        }
        filepath_size_buf[reader_delimeter(fd_client, filepath_size_buf, DELIMETER) - 1] = '\0';
        filepath_size = atoi(filepath_size_buf);
        char path[filepath_size + 1];
        path[reader(fd_client, path,filepath_size, filepath_size )] = '\0';
        char filesize_data[MESSAGE_STRING_SIZE];
        int file_size = get_filesize(path);
        if(file_size ==-1)
        {
          printf("bad file descriptor for size file function\n");
          return;
        }
        sprintf(filesize_data  ,"%d" ,file_size );
        char message_prep_data[MESSAGE_STRING_SIZE];
        message_prep_server(message_prep_data, strlen(filesize_data) + 2, MESSAGE_SUCCESS);
        char message[strlen(message_prep_data) + strlen(filesize_data) + 3];
        strcpy(message, message_prep_data);
        strcat(message, DELIMETER);
        strcat(message, filesize_data);
        strcat(message, DELIMETER);
        printf("writing this to client: %s\n", message );
        writer(fd_client, message, strlen(message), strlen(message));
        if(eval_command(OPEN, fd_client) != 0)
        {
          printf("invalid command\n");
          //let the client know about this error
          return;
        }
        filepath_size_buf[reader_delimeter(fd_client, filepath_size_buf, DELIMETER) - 1] = '\0';
        filepath_size = atoi(filepath_size_buf);
        char open_pathname[filepath_size];
        open_pathname[reader(fd_client, open_pathname ,filepath_size, filepath_size )] = '\0';
        flags_data[reader_delimeter(fd_client, flags_data, DELIMETER) - 1] = '\0';

        printf("calling net open...\n");
        file_descriptor  = netopen_server(open_pathname, fd_client ,atoi(flags_data) );
        if(file_descriptor ==-1)
        {
          printf("bad file descriptor");
          return;
        }
       printf("file descriptor on server side: %d\n", file_descriptor);
       char read_file_descriptor_data[MESSAGE_STRING_SIZE];
       char bytegoal_data[MESSAGE_STRING_SIZE];
       int byte_goal = 0;
       int read_file_descriptor = 0 ;
       int bytes_read  = 0;
       int total_bytes_read = 0;
       while(total_bytes_read<file_size)
       {
         if(eval_command(READ, fd_client) != 0)
         {
           printf("invalid command\n");
           //attempt to let the client know about this error
           return;
         }
         read_file_descriptor_data[reader_delimeter(fd_client, read_file_descriptor_data, DELIMETER) - 1] = '\0';
         bytegoal_data[reader_delimeter(fd_client, bytegoal_data, DELIMETER) - 1] = '\0';
         byte_goal = atoi(bytegoal_data);
         read_file_descriptor = atoi(read_file_descriptor_data);
         bytes_read =  netread_server(fd_client, read_file_descriptor * -1 , byte_goal);

         if(bytes_read == -1)
         {
           printf("something went wrong on netread_server\n");
           return ;
         }
         total_bytes_read += bytes_read;
         printf("total bytes read: %d and byte_goal: %d\n", total_bytes_read, byte_goal);
       }
       if(eval_command(CLOSE, fd_client) != 0)
       {
         printf("invalid command\n");
         //attempt to let the client know about this error
         return;
       }
        char close_file_descriptor_data[MESSAGE_STRING_SIZE];
        close_file_descriptor_data[reader_delimeter(fd_client, close_file_descriptor_data, DELIMETER) - 1] ='\0';
        int close_file_descriptor = atoi(close_file_descriptor_data);
         if(netclose_server(fd_client , close_file_descriptor * -1) == -1)
         {
           printf("something went wrong with netclose_server");
           return -1;
         }
         printf("This seems fine\n");
      }
      else
      {
        //error handling
      }


printf("calling pthread_exit\n: %d\n", pthread_self());

      pthread_exit(NULL);
 }

int netclose_server(int fd_client, int file_descriptor)
{
  if( close(file_descriptor) == -1)
  {
    //attempt to tell the client I screwed up, somehow
    perror("close");
    return -1;
  }
  char message_prep_data[MESSAGE_STRING_SIZE];
  char message[BUFFER_SIZE];
  message_prep_server(message_prep_data,strlen(BYE_CODE),MESSAGE_SUCCESS);
  strcpy(message, message_prep_data);
  strcat(message, DELIMETER);
  strcat(message, BYE_CODE);
  strcat(message, DELIMETER);
  printf("writing this client: %s", message);
  writer(fd_client, message, strlen(message), strlen(message));
  if( close(fd_client) == -1)
  {
    perror("closing socket");
    return -1;

  }
return 0;
}
char* read_message(int fd_client)
{
int byte_goal= sizeof_message(fd_client); //this turned out to be kind of useless
printf("byte goal: %d \n", byte_goal);
char* command = malloc(sizeof(char) * BUFFER_SIZE) ;
command[reader_delimeter(fd_client, command, DELIMETER)-1] = '\0';
//printf("strlen: %d\n", strlen(message_size));
printf("command: %s\n", command);
return command;
}

int interpret_message_server(int fd_client, char* command)
{
  //add one because split() ALWAYS returns a list terminated by a NULL pointer

 char  instruction[strlen(command) + 1];
 strcpy(instruction, command);
 printf("instruction: %s", instruction);

if(strcmp(instruction, OPEN) == 0)
{
  int file_descriptor  =0;
  int filepath_size = 0;
  int flags =0 ;
  char flags_data[MESSAGE_STRING_SIZE];
  char filepath_size_buf[MESSAGE_STRING_SIZE];
  char Throwaway_delimeter[1];
  filepath_size_buf[reader_delimeter(fd_client, filepath_size_buf, DELIMETER) - 1] = '\0';
  filepath_size = atoi(filepath_size_buf);
  printf("filepath_size: %d\n", filepath_size);
  char path[filepath_size + 1];
  path[reader(fd_client, path,filepath_size, filepath_size )] = '\0';
  reader_delimeter(fd_client, Throwaway_delimeter, DELIMETER );
  flags_data[reader(fd_client,flags_data, 1, 1)] = '\0';
  printf("net_open branch\n");
  netopen_server( path, fd_client, atoi(flags_data));
}
else if(strcmp(instruction, READ) == 0)
{
  printf("reading file\n");
  char file_descriptor_data[MESSAGE_STRING_SIZE];
  char server_byte_goal_data[MESSAGE_STRING_SIZE];
  file_descriptor_data[reader_delimeter(fd_client, file_descriptor_data, DELIMETER) - 1] = '\0';
  printf("file descrptor sent by the client: %s\n", file_descriptor_data);
  server_byte_goal_data[reader_delimeter(fd_client, server_byte_goal_data , DELIMETER) - 1] = '\0';
  printf("byte goal sent by the client: %s\n", server_byte_goal_data);

  netread_server(fd_client, atoi(file_descriptor_data) * -1, atoi(server_byte_goal_data));
}
else if(strcmp(instruction, WRITE) == 0)
{
  printf("writing file\n");
  char file_descriptor_data[MESSAGE_STRING_SIZE];
  char server_byte_goal_data[MESSAGE_STRING_SIZE];
  file_descriptor_data[reader_delimeter(fd_client, file_descriptor_data, DELIMETER) - 1] = '\0';
  server_byte_goal_data[reader_delimeter(fd_client, server_byte_goal_data , DELIMETER) - 1] = '\0';
  netwrite_server(fd_client, atoi(file_descriptor_data) * -1, atoi(server_byte_goal_data));
}
else if(strcmp(instruction, READ_AND_WRITE) == 0)
{
  //net_something(I'm not sure yet)
}
  return 0;
}
