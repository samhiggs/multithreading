all: solution.exe

test: test_env.exe

solution.exe: solution.o
	 gcc -Wall -pedantic -std=c11 -g solution.c logger.c -o binary -lpthread

solution.o: solution.c
	 gcc -c solution.c logger.c -lpthread
     
test_env.exe: test_env.o
	 gcc -Wall -pedantic -std=c11 test_env.c -o test -lpthread

test_env.o: test_env.c
	 gcc -c test_env.c -lpthread

clean:
	 rm solution.o solution.exe