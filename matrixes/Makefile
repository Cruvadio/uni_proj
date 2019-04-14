CC=g++
CFLAGS=-g

all: test

test: main.o libmatrix.a
	$(CC) $(CFLAGS) -o $@ $^

libmatrix.a: exception.o matrix.o rational.o readfile.o vector.o
	ar crv $@ $^
	ranlib $@
	#rm $^

%.o: %.cpp
	$(CC) $(CFLAGS) -c $^

clean:
	rm *.o
