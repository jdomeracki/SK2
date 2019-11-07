#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#define MAX_SIZE 8192
#define PORT 8080


//Testowe resposny

char OK[] = "HTTP/1.1 200 OK\r\n"
"Content-Type: text/html; charset=UTF-8\r\n\r\n"
"<!DOCTYPE html><html><head><title>Wersja pierwsza</title></head>"
"<body><h1>TEST</h1></body></html>\r\n";

char METHOD_NOT_ALLOWED[] = "HTTP/1.1 405 Method Not Allowed\r\n"
"Content-Type: text/html; charset=iso-8859-1\r\n\r\n"
"<!DOCTYPE HTML PUBLIC><html><head><title>405 Method Not Allowed</title></head>"
"<body><h1>Method Not Allowed</h1><p>Your browser sent a requst with a method that is not supported.</p></body></html>\r\n";


char BAD_REQUEST[] = "HTTP/1.1 400 Bad Request\r\n"
"Content-Type: text/html; charset=iso-8859-1\r\n\r\n"
"<!DOCTYPE HTML PUBLIC><html><head><title>400 Bad Request</title></head>"
"<body><h1>Bad Request</h1><p>Your browser sent a request that this server could not understand.</p><p>The request line contained invalid characters following the protocol string.</p></body></html>\r\n";

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
 
    while(1)
    {
    if (bind(srv_fd, (struct sockaddr *)&srv_adr, sin_len) < 0)
    {
        perror("Couldn't bind!");
        exit(1);
    }
    break;
    }
    
  if (listen(srv_fd, 7) < 0)
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
    
    char request[MAX_SIZE] = {0};
    char request_cp[MAX_SIZE] = {0};
    char *ptr, *method, *uri, *version;
    int req_len = read(cli_fd, request, MAX_SIZE);
    strcpy(request_cp, request);
    
    //tokenizacja requestu 
    ptr = strtok(request_cp, " \r\n");
    method = ptr;
    ptr = strtok(NULL, " \r\n");
    uri = ptr;
    ptr = strtok(NULL, " \r\n");
    version = ptr; 
 
    //podstawowa walidacja   
  if (strncmp(method, "GET", 3) != 0 )
    {
    puts("METHOD_NOT_ALLOWED");
    write(cli_fd, METHOD_NOT_ALLOWED, strlen(METHOD_NOT_ALLOWED));
    close(cli_fd);
    }
 else if (strncmp(version, "HTTP/1.1", 8) != 0 )
    {
    puts("HTTP VERSION NOT SUPPORTED");
    write(cli_fd, BAD_REQUEST, strlen(BAD_REQUEST));
    close(cli_fd);
   }
  else
    {  
    printf("%s\r\n", request);
    write(cli_fd, OK, strlen(OK));
    close(cli_fd);
    }
  }
}
