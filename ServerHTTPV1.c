#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MAX_SIZE 8192
#define PORT 8080
#define QUEUE_SIZE 1000

//Plik
struct DOCUMENT
{
char *file_path;
int file_path_len;
};

//Responsy
char OK[] = "HTTP/1.1 200 OK\r\n"
"Content-Type: text/html; charset=UTF-8\r\n\r\n";

char BAD_REQUEST[] = "HTTP/1.1 400 Bad Request\r\n"
"Content-Type: text/html; charset=iso-8859-1\r\n\r\n"
"<!DOCTYPE HTML PUBLIC><html><head><title>400 Bad Request</title></head>"
"<body><h1>Bad Request</h1><p>Your browser sent a request that this server could not understand.</p><p>The request line contained invalid characters following the protocol string.</p></body></html>\r\n";

char FILE_NOT_FOUND[] = "HTTP/1.1 404 File Not Found\r\n"
"Content-Type: text/html; charset=iso-8859-1\r\n\r\n"
"<!DOCTYPE HTML PUBLIC><html><head><title>404 File Not Found</title></head>"
"<body><h1>404 File Not Found</h1><p>File was not found.</p>\r\n";

char METHOD_NOT_ALLOWED[] = "HTTP/1.1 405 Method Not Allowed\r\n"
"Content-Type: text/html; charset=iso-8859-1\r\n\r\n"
"<!DOCTYPE HTML PUBLIC><html><head><title>405 Method Not Allowed</title></head>"
"<body><h1>Method Not Allowed</h1><p>Your browser sent a requst with a method that is not supported.</p></body></html>\r\n";

//Statyczna tablica plików
struct DOCUMENT TAB[] = {
{"/index", strlen("/index")},
{"/photo", strlen("/photo")},
{"/song", strlen("/song")}
};

//Sprawdzenie czy plik istnieje
int FileCheck(char *file_name)
{
int tab_size = 3;
    if (strncmp(file_name, "/\0", 2) == 0)
    {
    return 2;
    }
    for (int i = 0; i < tab_size; i++)
    {
        if(strncmp(file_name, TAB[i].file_path, TAB[i].file_path_len + 1) == 0)
        {
        return 1;
        }
    }
return 0;
}

//Zachowanie wątku
void *Thread(void *ptr_cli_fd)
{
    int cli_fd = *(int*)ptr_cli_fd;
    free(ptr_cli_fd);
    pthread_detach(pthread_self());
    char request[MAX_SIZE] = {0};
    char request_cp[MAX_SIZE] = {0};
    char *ptr, *method, *uri, *version;
    read(cli_fd, request, MAX_SIZE);
    int file_check_status;
    strcpy(request_cp, request);
    ptr = request_cp;
    
    //tokenizacja requestu 
    method = strtok_r(ptr, " \r\n", &ptr);
    uri = strtok_r(ptr, " \r\n", &ptr);
    version = strtok_r(ptr, " \r\n", &ptr);
    file_check_status = FileCheck(uri);
    
    
    //podstawowa walidacja   
  if (strncmp(method, "GET", 3) != 0 )
    {
    puts("METHOD_NOT_ALLOWED");
    write(cli_fd, METHOD_NOT_ALLOWED, strlen(METHOD_NOT_ALLOWED));
    close(cli_fd);
    return 0;
    }
    else if (file_check_status == 0)
    {
    puts("FILE NOT FOUND");
    write(cli_fd, FILE_NOT_FOUND, strlen(FILE_NOT_FOUND));
    close(cli_fd);
    return 0;
   }
  else if (strncmp(version, "HTTP/1.1", 8) != 0 )
    {
    puts("HTTP VERSION NOT SUPPORTED");
    write(cli_fd, BAD_REQUEST, strlen(BAD_REQUEST));
    close(cli_fd);
    return 0;
   }
  else
    {  
    if(file_check_status == 2)
    {
    FILE *f = fopen("index", "r");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET); 
    char *string = malloc(fsize + 2);
    fread(string, 1, fsize, f);
    fclose(f);
    puts("OK");
    char *response = malloc(strlen(OK) + strlen(string + 2));
   
    strcpy(response, OK);
    strcat(response, string);
    strcat(response, "\r\n");
     
    write(cli_fd, response, strlen(response));
    close(cli_fd);
    }
    else if(file_check_status == 1)
    {
    char *file_path = uri+1;
    FILE *f = fopen(file_path, "r");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET); 
    char *string2 = malloc(fsize + 2);
    fread(string2, 1, fsize, f);
    fclose(f);
    puts("OK");
    char *response2 = malloc(strlen(OK) + strlen(string2 + 2));
    
    strcpy(response2, OK);
    strcat(response2, string2);
    strcat(response2, "\r\n");
    
    write(cli_fd, response2, strlen(response2));
    close(cli_fd);
    }
    return 0;
    }
}


int main(int argc, char const *argv[])
{
  struct sockaddr_in srv_adr;
  int srv_fd, cli_fd;
  int sin_len = sizeof(srv_adr);
     
  if ((srv_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("Couldn't create socket!");
        exit(1);
    }
    
  srv_adr.sin_family = AF_INET;
  srv_adr.sin_addr.s_addr = INADDR_ANY;
  srv_adr.sin_port = htons(PORT);
  memset(srv_adr.sin_zero, '\0', sizeof(srv_adr.sin_zero));
 
 
  if (bind(srv_fd, (struct sockaddr *)&srv_adr, sin_len) < 0)
    {
        perror("Couldn't bind!");
        exit(1);
    }
    
  if (listen(srv_fd, QUEUE_SIZE) < 0)
    {
        perror("Can't listen!");
        exit(1);
    }
 
  while (1)
  {
  
  if ((cli_fd = accept(srv_fd, (struct sockaddr *)&srv_adr, (socklen_t*)&sin_len)) < 0)
    {
        perror("Couldn't accept the connection!");
        exit(1);
    }
    
   pthread_t t;
   int *ptr_cli_fd = malloc(sizeof(int));
   *ptr_cli_fd = cli_fd;
   pthread_create(&t, NULL, Thread, ptr_cli_fd);
      
  }
  
  return 0;
}
