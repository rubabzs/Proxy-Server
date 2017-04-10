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

struct arg_struct {
  string path;
  int socket_fd;
  int threads;
};

void *myClientFunc(void *arguments) {

  struct arg_struct *args = (struct arg_struct *)arguments;
  int bytes_recv, total_bytes_recv;
  char buffer[COMBUF];
  char recv_buff[RESBUF];
  // variables are defined here

  int no_of_requests = args->threads;
  memset(recv_buff, '0', sizeof(recv_buff));
  for (int j = 0;j < no_of_requests;j++)
    {
      sprintf(buffer, "GET /%s HTTP/1.1\r\n", (args->path).c_str());
      cout << "\n\n*****************************************************\n";
      cout << buffer; 
      send((args->socket_fd), buffer, strlen(buffer), 0);
      bzero(buffer, sizeof(buffer));
      bytes_recv = 0;
      total_bytes_recv = 0;
      cout << "*****************************************************\n";
      while((bytes_recv = recv((args->socket_fd), recv_buff, strlen(recv_buff), 0)) > 0){
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
    close(args->socket_fd);
  return NULL;
}


int main(int argc, char *argv[])
{
  int i, no_of_threads;
  no_of_threads = atoi(argv[1]);
  pthread_t tid[no_of_threads];
  struct arg_struct args;
  
  int socket_fd, total_bytes_recv, port_no;
  struct sockaddr_in server_addr;
  struct hostent *host_info;
  struct in_addr **ip_addr_list;
  string URL, hostname, port, proxy_addr, proxy_port, path;

  if (argc < 3) {
    fprintf(stderr, "Usage: myclient <no of threads> [-proxy proxy_addr] <URL>: Success\n\n");
    exit(-1);
  }
  
  if (strcmp(argv[2], "-proxy") == 0) {
    URL = argv[4];
    path = URL.substr(7, URL.find("\n"));
    proxy_addr = argv[3];
    port = proxy_addr.substr(10, 4);
    port_no = atoi(port.c_str()); 
    hostname = proxy_addr.substr(0, 9);
    host_info = gethostbyname(hostname.c_str());
  }
  else {
    URL = argv[2];
    path = URL.substr(21, URL.find("\n"));
    port = URL.substr(17, 4);
    port_no = atoi(port.c_str()); 
    hostname = URL.substr(7, 9);
    host_info = gethostbyname(hostname.c_str());
  }  

  // Code
  cout << "Running client...\n";
  
  if ((host_info == NULL) || (port_no <= 1024) || (port_no >= 65535)) {
    fprintf(stderr, "Usage: myclient <no of threads> [-proxy proxy_addr] <URL>: Success\n\n");
    exit(-1);
  }

  memset(&server_addr, '0', sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port_no);

  ip_addr_list = (struct in_addr **) host_info->h_addr_list;
  inet_pton(AF_INET, inet_ntoa(*ip_addr_list[0]), &server_addr.sin_addr);
  cout << "hostname: " << hostname << " has IP: " << inet_ntoa(*ip_addr_list[0]) << "\n\n"; 

  // Creating socket
  cout << ".. creating local connector socket\n";
  socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd < 0) {
    cout << "Error: Socket could not be created\n";
    return NULL;
  }

  // Establishing connection 
  cout << ".. connecting socket to " << hostname << ":" << port << "\n";
  if (connect(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
    cout << "Connection error... exiting: Connection failed\n";
    return NULL;
  }

  args.path = path;
  args.socket_fd = socket_fd;
  args.threads = atoi(argv[1]);
  // Let us create threads
  
  for (i = 0; i < no_of_threads; i++)
    pthread_create(&tid[i], NULL, myClientFunc, (void *) &args);
  
  for (i = 0; i < no_of_threads; i++)
    pthread_join(tid[i], NULL);

  pthread_exit(NULL);
  return 0; 
}
