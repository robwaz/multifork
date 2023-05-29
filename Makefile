all: multifork.o example

multifork.o: multifork.c
	gcc -I. -masm=intel -g -pthread -c -fPIC multifork.c

example: example.c
	gcc -I. -g -masm=intel -pthread example.c *.o

clean:
	rm ./a.out multifork.o

