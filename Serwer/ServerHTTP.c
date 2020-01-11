#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MAX_SIZE 8192
#define PORT 12345
#define QUEUE_SIZE 1000

//Struktura do obsługi pliku
struct DOCUMENT
{
char *file_path;
int file_path_len;
};

//Responsy
char OK[] = "HTTP/1.1 200 OK\r\n"
"Server: SK2\r\n"
"Content-Type: text/html; charset=UTF-8\r\n\r\n";

char BAD_REQUEST[] = "HTTP/1.1 400 Bad Request\r\n"
"Server: SK2\r\n"
"Content-Type: text/html; charset=iso-8859-1\r\n\r\n"
"<!DOCTYPE HTML PUBLIC><html><head><title>400 Bad Request</title></head>"
"<body><h1>400 Bad Request</h1><p>Your browser sent a request that this server could not understand.</p>"
"<p>The request line contained invalid characters following the protocol string.</p></body></html>\r\n";

char FILE_NOT_FOUND[] = "HTTP/1.1 404 File Not Found\r\n"
"Server: SK2\r\n"
"Content-Type: text/html; charset=iso-8859-1\r\n\r\n"
"<!DOCTYPE HTML PUBLIC><html><head><title>404 File Not Found</title></head>"
"<body><h1>404 File Not Found</h1><p>File was not found.</p>\r\n";

char METHOD_NOT_ALLOWED[] = "HTTP/1.1 405 Method Not Allowed\r\n"
"Server: SK2\r\n"
"Content-Type: text/html; charset=iso-8859-1\r\n\r\n"
"<!DOCTYPE HTML PUBLIC><html><head><title>405 Method Not Allowed</title></head>"
"<body><h1>405 Method Not Allowed</h1><p>Your browser sent a requst with a method that is not supported.</p></body></html>\r\n";

//Statyczna tablica plików
struct DOCUMENT TAB[] = {
{"/index", strlen("/index")},
{"/photo", strlen("/photo")},
{"/song", strlen("/song")}
};


//Sprawdzenie czy metoda jest wspierana
int MethodCheck(char *method)
{
if (strncmp(method, "GET", 3) == 0)
return 1;
else 
return 0;
}

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

//Sprawdzenie czy wersja HTTP jest obsługiwana
int VersionCheck(char *version)
{
if (strncmp(version, "HTTP/1.1", 8) == 0 )
  return 1;
else 
  return 0;
}

//Sprawdzenie czy udało się wpisać do socketu
void WriteCheck(int cli_fd, char *resposne)
{
int num_of_bytes_written = 0; 
do
    {
    num_of_bytes_written += write(cli_fd, resposne, strlen(resposne));
    if(num_of_bytes_written <= 0) 
    {
      perror("Couldn't write to the socket!");
      exit(1);}
    } while (num_of_bytes_written != strlen(resposne));
}

//Sprawdzenie czy udało się odczytać z socketu
void ReadCheck(int cli_fd, char *request)
{
if (read(cli_fd, request, MAX_SIZE) == -1)
    {
        perror(strerror(errno));
        exit(1);
    }
}

//Odczyt danych z pliku i utworzenie response
char *ResponseCompose(char *uri)
{
  FILE *f = fopen(uri, "r");
  fseek (f, 0, SEEK_END);
  long fsize = ftell(f);
  fseek(f, 0, SEEK_SET); 
  char *string = malloc(fsize + 2);
  fread(string, 1, fsize, f);
  fclose(f);
  char *response = malloc(strlen(OK) + strlen(string + 2));
  strcpy(response, OK);
  strcat(response, string);
  strcat(response, "\r\n");
  free(string);
  return response;
}


//Zachowanie wątku
void *Thread(void *ptr_cli_fd)
{
    int cli_fd = *(int*)ptr_cli_fd;
    free(ptr_cli_fd);
    pthread_detach(pthread_self());
    char request[MAX_SIZE] = {0};
    char request_cp[MAX_SIZE] = {0};
    char *ptr, *method, *uri, *version, *response, *response_defualt;
    int file_check_status, method_check_status, version_check_status;
    ReadCheck(cli_fd, request);
    strcpy(request_cp, request);
    ptr = request_cp;
    method = strtok_r(ptr, " \r\n", &ptr);
    uri = strtok_r(ptr, " \r\n", &ptr);
    version = strtok_r(ptr, " \r\n", &ptr);
    method_check_status = MethodCheck(method);
    file_check_status = FileCheck(uri);
    version_check_status = VersionCheck(version);

    //Podstawowa walidacja
    switch (method_check_status)
    {
    case 0:
    //puts("METHOD NOT ALLOWED");
    WriteCheck(cli_fd, METHOD_NOT_ALLOWED);
      break;

    case 1:
          switch (version_check_status)
          {
          case 0:
            //puts("BAD REQUEST");
            WriteCheck(cli_fd, BAD_REQUEST);
          break;

          case 1:
                switch (file_check_status)
                {
                case 0:
                  //puts("FILE NOT FOUND");
                  WriteCheck(cli_fd, FILE_NOT_FOUND);
                break;
                
                case 1:
                  //puts("OK");
                  response = ResponseCompose(uri+1);
                  WriteCheck(cli_fd, response);
                  free(response);
                 break; 

                case 2:
                  //puts("OK");
                  response_defualt = ResponseCompose("index");
                  WriteCheck(cli_fd, response_defualt);
                  free(response_defualt);
                break;
                
                default:
                  break;
          }
          
          default:
            break;
          }
    
    default:
      break;
    }
    close(cli_fd);
    return 0;
}


int main(int argc, char const *argv[])
{
  struct sockaddr_in srv_adr;
  int srv_fd, cli_fd;
  int sin_len = sizeof(srv_adr);
  int reu_adr_val = 1;
     
  if ((srv_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("Couldn't create socket!");
        exit(1);
    }
    
  srv_adr.sin_family = AF_INET;
  srv_adr.sin_addr.s_addr = INADDR_ANY;
  srv_adr.sin_port = htons(PORT);
  memset(srv_adr.sin_zero, '\0', sizeof(srv_adr.sin_zero));
  setsockopt(srv_fd , SOL_SOCKET, SO_REUSEADDR, (char*)&reu_adr_val, sizeof(reu_adr_val));
 
 
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

  puts ("Server SK2 is running...");
  puts ("PORT# -> 12345");
  puts ("Accepted HTTP methods -> GET");

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
