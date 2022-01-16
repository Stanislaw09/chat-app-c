CC = gcc
CFLAGS  = -g

default: client server

client:  client.o common.o
	$(CC) $(CFLAGS) -o client client.o common.o
server:  server.o common.o
	$(CC) $(CFLAGS) -o server server.o common.o

server.o:  server.c common.c common.h const.h
	$(CC) $(CFLAGS) -c server.c
client.o:  client.c common.c common.h const.h
	$(CC) $(CFLAGS) -c client.c
common.o:  common.c common.h const.h
	$(CC) $(CFLAGS) -c common.c

clean: 
	$(RM) client *.o *~
	$(RM) server *.o *~
	rm -rf ./temp
	ipcs -q | tr -s ' ' | cut -d' ' -f2 | tail -n +4 | xargs -r ipcrm msg