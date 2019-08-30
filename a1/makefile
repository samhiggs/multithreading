all: solution.exe

solution.exe: solution.o
	 gcc -Wall -pedantic -std=c11 solution.c logger.c -o binary -lpthread

solution.o: solution.c
	 gcc -c solution.c logger.c -lpthread
     
clean:
	 rm solution.o solution.exe