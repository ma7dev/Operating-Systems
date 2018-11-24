CC = gcc
CFLAGS  =-Wall -g -pthread

default: concurrency

concurrency:  producer.o consumer.o task.o main.o rand.o
	$(CC) $(CFLAGS) -o concurrency producer.o consumer.o task.o main.o rand.o

rand.o:  rand.c rand.h
	$(CC) $(CFLAGS) -c rand.c

main.o:  main.c consumer.h producer.h task.h rand.h
	$(CC) $(CFLAGS) -c main.c

producer.o:  producer.c producer.h task.h rand.h
	$(CC) $(CFLAGS) -c producer.c

consumer.o:  consumer.c consumer.h task.h rand.h
	$(CC) $(CFLAGS) -c consumer.c

task.o:  task.c task.h
	$(CC) $(CFLAGS) -c task.c

clean: 
	$(RM) concurrency *.o *~