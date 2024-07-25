CC=gcc
CFLAGS=-g -Wall -O3 -ffast-math  
LDFLAGS=-g -fopenmp
OBJECTS=$(SOURCES:.c=.o)

bucket:
	$(CC) $(LDFLAGS) $^-o $@ bucket_s.c -lm

all: bucket

%.o: %.c
	$(CC) $(CFLAGS) -c $<  $@

clean: 
	rm bucket