all: client.o client server.o server proxy_server.o proxy_server

client.o : client.cc
	g++ -W -Wall -c client.cc
client : client.o 
	g++ -W -Wall client.o -pthread -o client
server.o : server.c
	gcc -W -Wall -c server.c
server : server.o 
	gcc -W -Wall server.o -pthread -o server	
proxy_server.o : proxy_server.c
	gcc -W -Wall -c proxy_server.c
proxy_server : proxy_server.o 
	gcc -W -Wall proxy_server.o -pthread -o proxy_server	
clean : 
	rm server server.o client.o client proxy_server proxy_server.o

