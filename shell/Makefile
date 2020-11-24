CC = gcc
CFLAGS = -g -pedantic

shell: str.o processes.o shell.o regex.o
	$(CC) -o $@ $^

processes.o: processes.c
	$(CC) $(CFLAGS) -c $^
shell.o: shell.c
	$(CC) $(CFLAGS) -c $^
str.o: str.c
	$(CC) $(CFLAGS) -c $^
regex.o: regex.c
	$(CC) $(CFLAGS) -c $^
clean:
	rm shell.o processes.o str.o regex.o
