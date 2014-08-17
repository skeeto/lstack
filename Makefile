CC      = /usr/lib/gcc-snapshot/bin/gcc
CFLAGS  = -std=c11 -Wall -O3 -I/usr/include
LDFLAGS = -pthread

main : main.o lstack.o sha1.o

main.o : main.c lstack.h sha1.h
lstack.o : lstack.c lstack.h
sha1.o : sha1.c

.PHONY : run clean

run : main
	./$^

clean :
	$(RM) main *.o
