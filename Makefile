all: client.o client server.o server

client.o : client.cc
	g++ -W -Wall -c client.cc
client : client.o 
	g++ -W -Wall client.o -pthread -o client
server.o : server.c
	gcc -W -Wall -c server.c
server : server.o 
	gcc -W -Wall server.o -pthread -o server	
clean : 
	rm server server.o client.o client 

