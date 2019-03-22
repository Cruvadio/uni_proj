CC=g++
CFLAGS=-g

all: test

test: *.o
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.cpp
	$(CC) $(CFLAGS) -c $^

clean:
	rm *.o
