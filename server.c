#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>
#include <strings.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <pthread.h>
#include "queue.c"

pthread_cond_t  workers = PTHREAD_COND_INITIALIZER;
pthread_cond_t  boss = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

#define MAX_THREADS 2000
#define ISspace(x) isspace((int)(x))

void process_request(int);
void error_handle(const char *);
int get_line(int, char *, int);
void file_not_found(int);
void send_file(int, const char *);
int bootstrap(int);
void* worker_function(void *);

// main worker function to handle requests
void* worker_function(void *c) {
  int client_sock;
  while(1) {
    pthread_mutex_lock(&mutex);
    while(empty()) {
      printf("Queue is empty, this thread is going to wait\n");
      pthread_cond_wait(&workers, &mutex);
    }
    printf("Going to process request.\n");
    client_sock = deq();
    
    if (queuesize() < MAX_THREADS) 
      pthread_cond_signal(&boss);

    pthread_mutex_unlock(&mutex);
    process_request(client_sock);
  }
  return NULL;
}

// process accepted request
void process_request(int client)
{
  char buf[1024];
  //int n;
  char method[255];
  char url[255];
  char path[512];
  size_t i, j;
  struct stat st;
 
  get_line(client, buf, sizeof(buf));
  
  i = 0; j = 0;
  while (!ISspace(buf[j]) && (i < sizeof(method) - 1)) {
    method[i] = buf[j];
    i++; j++;
  }
  method[i] = '\0';
//read method upto this
  i = 0;
//pass spaces
  while (ISspace(buf[j]) && (j < sizeof(buf)))
    j++;  
  while (!ISspace(buf[j]) && (i < sizeof(url) - 1) && (j < sizeof(buf))) {
    url[i] = buf[j];
    i++; j++;
  }
  url[i] = '\0';
  printf("url: %s", url);
  sprintf(path, "files%s", url);
  if (path[strlen(path) - 1] == '/')
    strcat(path, "index.html");
  if (stat(path, &st) == -1) {
    file_not_found(client);
  }
  else {
    if ((st.st_mode & S_IFMT) == S_IFDIR)
      strcat(path, "/index.html");
    send_file(client, path);
 }
  printf("Closing client socket\n");
  close(client);
}


// handle error
void error_handle(const char *error)
{
  perror(error);
  exit(1);
}


// reads data from client socket
int get_line(int sock, char *buf, int size)
{
  int i = 0;
  char c = '\0';
  int n;

  while ((i < size - 1) && (c != '\n')) {
    n = recv(sock, &c, 1, 0);
    if (n > 0)
      {
	if (c == '\r')
	  {
	    n = recv(sock, &c, 1, MSG_PEEK);
	    if ((n > 0) && (c == '\n'))
	      recv(sock, &c, 1, 0);
	    else
	      c = '\n';
	  }
	buf[i] = c;
	i++;
      }
    else
      c = '\n';
  }
 buf[i] = '\0';
 printf("buffer: %s\n", buf);
 return(i);
}


// making headers for response
void make_header(int client, const char *filename)
{
 char buf[1024];
 (void)filename;  // why void?

 strcpy(buf, "HTTP/1.1 200 OK\r\n");
 send(client, buf, strlen(buf), 0);
}


// 404: file not found
void file_not_found(int client)
{
 char buf[1024];

 sprintf(buf, "HTTP/1.1 404 NOT FOUND\r\n");
 send(client, buf, strlen(buf), 0);
}


// send file to client
void send_file(int client, const char *filename)
{
 FILE *resource = NULL;
 char buf[1024];
 char send_buf[1024];

 buf[0] = 'A'; buf[1] = '\0';

 resource = fopen(filename, "r");
 if (resource == NULL)
  file_not_found(client);
 else
 {
   make_header(client, filename);
   fgets(buf, sizeof(buf), resource);
   while (!feof(resource)) {
     send(client, send_buf, strlen(send_buf), 0);
     fgets(send_buf, sizeof(send_buf), resource);
   }
 }
 fclose(resource);
}


// start listening on a specific port
int bootstrap(int port)
{
 int server_sock = 0;
 struct sockaddr_in name;

 server_sock = socket(PF_INET, SOCK_STREAM, 0);
 if (server_sock == -1)
   error_handle("socket"); 
 memset(&name, 0, sizeof(name));
 name.sin_family = AF_INET;
 name.sin_port = htons(port);
 name.sin_addr.s_addr = htonl(INADDR_ANY);

 if (bind(server_sock, (struct sockaddr *)&name, sizeof(name)) < 0)
  error_handle("bind");
 
 // listen pool check
 if (listen(server_sock, MAX_THREADS) < 0)
  error_handle("listen");
 return(server_sock);
}


int main(int argc, char *argv[])
{
 int server_sock = -1; 
 int client_sock = -1;
 int port, index;
 int thread_num;
 pthread_attr_t tattr;
 struct sockaddr_in client_name;
 socklen_t client_name_len = sizeof(client_name);
 signal(SIGPIPE,SIG_IGN); 

 if (argc != 3 || (atoi(argv[1]) < 1024 || atoi(argv[1]) > 65535)) {
   fprintf(stderr, "Usage: server <port number between 1024 to 65535> <number of worker threads>\n");
   exit(-1);
 }

 port = atoi(argv[1]); 
 thread_num = atoi(argv[2]);
 pthread_t threads[thread_num];
 pthread_attr_init(&tattr); //scope in not being used
 pthread_attr_setscope(&tattr, PTHREAD_SCOPE_SYSTEM); 
 server_sock = bootstrap(port);
 printf("web server running on port %d with pool of %d threads\n", port, thread_num);

 //initializing pool of threads
 for (index=0; index<thread_num; index++) {
   if (pthread_create(&threads[index], NULL, &worker_function, NULL) != 0)
     error_handle("pthread create");                 
 }
 printf("%d Threads successfully created\n", index-1);
 
 // main loop of boss to accept connections
 while (1) {
   
   client_sock = accept(server_sock, (struct sockaddr *)&client_name, &client_name_len);
   if (client_sock == -1)
     error_handle("accept");
   printf("New connection accepted\n");
   pthread_mutex_lock(&mutex);

   while(queuesize() == MAX_THREADS) {
     printf("Queue full, waiting for threads to consume stuff\n");
     pthread_cond_wait(&boss, &mutex);
   }
   printf("Adding client_sock to queue\n");
   enq(client_sock);
   pthread_cond_broadcast(&workers);
   pthread_mutex_unlock(&mutex);
 }

 close(server_sock);
 
 return(0);
}

