cc = gcc
CFLAGS = -g

all: mtr_creator mtr_reader
mtr_creator: crt_matrix.c
	$(cc) -o $@ $(CFLAGS) $^
mtr_reader: prog.c
	$(cc) -o $@ $(CFLAGS) $^
