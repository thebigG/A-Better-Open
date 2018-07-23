#ifndef LIBNETFILES_H
#define LIBNETFILES_H
#define HANDSHAKE "Deadpool's Handshake"
#define READ "r"
#define WRITE "w"
#define READ_AND_WRITE "rw"
#define OPEN "o"
#define CLOSE "c"
#define DELIMETER "|"
#define WORKING_DIRECTORY "./test_files"
#define MESSAGE_SUCCESS "s"
#define MESSAGE_ERROR "e"
#define SIZE_OF_FILE "sz"
#define CAT_NET "ct"
#define UPLOAD "up"
#define DOWNLOAD "down"
#define BYE_CODE "b"
#define BYE "Have a nice day!"
#define SERVER_ERROR -1
#define MESSAGE_STRING_SIZE 16
#define NUM_NETOPEN_TOKENS 5
#define BUFFER_SIZE 512
int netserverinit_server(const char* , struct addrinfo** , struct addrinfo*);
int netopen_server(const char* , int , int  );
char* getSubStr(const char* , int , int);
int count_tokens(char*, char*);
char** split(char*, char* );
char* read_message(int );
int interpret_message_server(int , char* );
int reader(int  ,char* , int , int);
int reader_delimeter(int  ,char* , char* );
int writer(int , char* , int , int );
int sizeof_message(int );
void message_prep_server(char* , int , const char* );
int netread_server(int , int , int );
void handle_request(void* );
int eval_command(char* , int);
int netclose_server(int , int);
#endif
