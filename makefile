CC = g++ 
CPPFLAGS = -std=c++14 -O2 -I. -I/usr/include/flint/
OBJS = main.o count.o
LIBRARIES = -L/usr/lib64/beignet/ -lcl -lflint

.PHONY : all, clean

all : main

main : $(OBJS)
	$(CC) $(CFLAGS) -o main $(OBJS) $(LIBRARIES)

clean :
	rm -f main main.o count.o
	
	
