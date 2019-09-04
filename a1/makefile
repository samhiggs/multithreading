all: binary.exe

CC=gcc
CFLAGS=-g -Wall -pedantic -std=c11 
PFLAG=-lpthread
DEPS=logger.h

binary.exe: solution.o logger.o
	$(CC) $(CFLAGS) -o binary solution.c logger.c $(PFLAG)

solution.o: solution.c
	$(CC) $(CFLAGS) -c solution.c $(PGLAG)

logger.o: logger.c
	$(CC) $(CFLAGS) -c logger.c $(PFLAG)

clean:
	del solution.o logger.o binary.exe