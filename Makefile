
OBJS1 	= dataServer.o producer-consumer.o 
OBJS	= remoteClient.o 
SOURCE1	= dataServer.c producer-consumer.c
SOURCE	= remoteClient.c 
HEADER  = producer-consumer.h 
OUT1  	= dataServer
OUT	= remoteClient
CC	= gcc
FLAGS   = -g -c 
FLAGS1  = -pthread -g -c


all: server client

server:$(OBJS1)
	$(CC) -pthread -g -o $(OUT1) $(OBJS1) 

client:$(OBJS)
	$(CC) -g -o $(OUT) $(OBJS)

 
producer-consumer.o: producer-consumer.c
	$(CC) $(FLAGS1) producer-consumer.c

dataServer.o: dataServer.c
	$(CC) $(FLAGS1) dataServer.c

remoteClient.o: remoteClient.c
	$(CC) $(FLAGS) remoteClient.c


clean:
	rm -f $(OBJS) $(OUT) $(OBJS1) $(OUT1)


