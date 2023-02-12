CFLAGS= -fPIC -g -O0

all: lib
lib: my_malloc.o
	gcc $(CFLAGS) -shared -o libmymalloc.so my_malloc.o
%.o: %.c my_malloc.h
	gcc $(CFLAGS) -c -o $@ $< 
clean:
	rm -f *~ *.o *.so
clobber:
	rm -f *~ *.o

