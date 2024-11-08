CC = gcc
LD = gcc
CFLAGS = -Wall -O3 -ffast-math -march=native
#CFLAGS = -Wall -g
LDFLAGS = -fopenmp
RM = /bin/rm -f
OBJS = main.o sort_funcs.o
EXECUTABLE = sort

all:$(EXECUTABLE)

$(EXECUTABLE): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $(EXECUTABLE) -lm

sort_funcs.o: sort_funcs.h sort_funcs.c
	$(CC) $(CFLAGS) $(LDFLAGS) -c sort_funcs.c

main.o: main.c sort_funcs.h
	$(CC) $(LDFLAGS) $(CFLAGS) -c main.c 

clean:
	$(RM) $(EXECUTABLE) $(OBJS)
