#ifndef LIBNETFILES_H
#define LIBNETFILES_H
#define HANDSHAKE "Deadpool's Handshake"
#define READ "r"
#define WRITE "w"
#define READ_AND_WRITE "rw"
#define OPEN "o"
#define CLOSE "c"
#define DELIMETER "|"
#define BYE_CODE "b"
#define BYE "Have a nice day!"
#define MESSAGE_SUCCESS "s"
#define MESSAGE_ERROR "e"
#define MESSAGE_STRING_SIZE 16
#define CAT_NET "ct"
#define CAT_COMMAND "-cat_net"
#define DOWNLOAD_COMMAND "-download"
#define UPLOAD_COMMAND "-upload"
#define UPLOAD "up"
#define DOWNLOAD "down"
#define BUFFER_SIZE 512
#define CLIENT_ERROR -1
#define STD_OUT 1
#define SIZE_OF_FILE "sz"
#define DOWNLOAD_PATH "download_"
#define WORKING_DIRECTORY "./test_files"
int netserverinit(const char *, const char*, struct addrinfo** , struct addrinfo* );
int netopen(const char *, int );
int get_io_flags(char* );
int interpret_message();
char  status_of_message();
int sizeof_message();
void message_prep(char* , int );
ssize_t netread(int , void*, size_t );
ssize_t netwrite(int , const void *, size_t);
char* get_command(char* );
int send_request(char* , char* );
#endif
