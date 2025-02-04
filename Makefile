CFLAGS = -Wall -Wextra -ggdb 

all:
	gcc -o hashtable hashtable.c hashtable.h fnv1a.h $(CFLAGS)

run:
	./hashtable
