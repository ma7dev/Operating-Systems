CC = gcc
CFLAGS  = -g -Wall

default: concurrency

concurrency:  producer.o consumer.o main.o
	$(CC) $(CFLAGS) -o concurrency producer.o consumer.o main.o

main.o:  main.c producer.h consumer.h rand.h task.h
	$(CC) $(CFLAGS) -c main.c

producer.o:  producer.c producer.h rand.h task.h
	$(CC) $(CFLAGS) -c producer.c

consumer.o:  consumer.c consumer.h rand.h task.h
	$(CC) $(CFLAGS) -c consumer.c

clean: 
	$(RM) concurrency *.o *~