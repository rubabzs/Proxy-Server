#include <stdio.h>      // basic I/O
#include <stdlib.h>
#include <iostream>     // basic I/O
#include <sys/types.h>  // standard system types
#include <netinet/in.h> // Internet address structures
#include <sys/socket.h> // socket API
#include <arpa/inet.h>
#include <netdb.h>      // host to IP resolution
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

using namespace std;

#define RESBUF 1024 	// maximum response size; can make it variable
#define COMBUF 256  	// maximum command size; can make it variable
int total_bytes = 100;

void *myClientFunc(void *args)
{
  char **argv = (char**)args; 
  
  // variables are defined here
  int socket_fd, bytes_recv, total_bytes_recv, port_no, file_index;
  char recv_buff[RESBUF];
  struct sockaddr_in server_addr;
  struct hostent *host_info;
  struct in_addr **ip_addr_list;
  char buffer[COMBUF];
  int  argc, diff, no_of_requests;
 
  // check that there are enough parameters
  argc = atoi(argv[3]);
  if (argc < 5) {
    fprintf(stderr, "Usage: myclient <hostname> <port number between 1024 and 65535> <no of threads> <no of requests> <set of files>: Success\n\n");
    exit(-1);
  }

  // Code
  cout << "Running client...\n";
  cout << "Will try to connect to " << argv[1] << " at port " << argv[2] << "\n\n";  
   
  // string to an integer
  port_no = atoi(argv[2]); 

  // Resolving Server IP Address
  host_info = gethostbyname(argv[1]);

  if ((host_info == NULL) || (port_no <= 1024) || (port_no >= 65535)) {
    fprintf(stderr, "Usage: myclient <hostname> <port number between 1024 and 65535> <no of threads> <no of requests> <set of files>: Success\n\n");
    exit(-1);
  }

  memset(recv_buff, '0', sizeof(recv_buff));
  memset(&server_addr, '0', sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port_no);

  ip_addr_list = (struct in_addr **) host_info->h_addr_list;
  inet_pton(AF_INET, inet_ntoa(*ip_addr_list[0]), &server_addr.sin_addr);
  cout << "hostname: " << argv[1] << " has IP: " << inet_ntoa(*ip_addr_list[0]) << "\n\n"; 

  // Creating socket
  cout << ".. creating local connector socket\n";
  socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd < 0) {
    cout << "Error: Socket could not be created\n";
    return NULL;
  }

  // Establishing connection 
  cout << ".. connecting socket to " << argv[1] << ":" << argv[2] << "\n";
  if (connect(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
    cout << "Connection error... exiting: Connection failed\n";
    return NULL;
  }

  // send multiple requests to the server
  no_of_requests = atoi(argv[4]);
  for (int j = 0;j < no_of_requests;j++)
    {
      // find random file from the set of files 
      diff = argc - 6;
      file_index =  (int) (((double)(diff+1)/RAND_MAX) * rand() + 5);
      sprintf(buffer, "GET /%s HTTP/1.1\r\n", argv[file_index]);
      cout << "\n\n*****************************************************\n";
      cout << buffer; 
      send(socket_fd, buffer, strlen(buffer), 0);
      bzero(buffer, sizeof(buffer));
      bytes_recv = 0;
      total_bytes_recv = 0;
      cout << "*****************************************************\n";
      while((bytes_recv = recv(socket_fd, recv_buff, strlen(recv_buff), 0)) > 0){
	total_bytes_recv = bytes_recv + total_bytes_recv;
	recv_buff[bytes_recv] = '\0';
	cout << recv_buff;
	memset(recv_buff, '0', sizeof(recv_buff));
      }
      cout << "*****************************************************\n";
      if (total_bytes_recv < 0) {
	cout << "Error: Server not responding\n";
      }
      else if (total_bytes_recv == 0) {
        cout << "Bytes received by client: " << total_bytes;
	cout << "\n*****************************************************\n";
      } 
      else {
	cout << "Bytes received by client: " << total_bytes_recv;
	cout << "\n*****************************************************\n";
      }
    }
  close(socket_fd);
  return NULL;
}


int main(int argc, char *argv[])
{
  int i, no_of_threads;
  no_of_threads = atoi(argv[3]);
  pthread_t tid[no_of_threads];
  srand(time(NULL));
 
  // replacing no_of_threads argument with argc (total number of arguments)
  sprintf(argv[3], "%d", argc);
     
  // Let us create threads
  for (i = 0; i < no_of_threads; i++)
    pthread_create(&tid[i], NULL, myClientFunc, (void *) argv);
  
  for (i = 0; i < no_of_threads; i++)
    pthread_join(tid[i], NULL);

  pthread_exit(NULL);
  return 0; 
}