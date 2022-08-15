# Code from https://github.com/Ph-k/quic. Philippos Koumparos (github.com/Ph-k)

CC = gcc
objects = quic.o copiers.o utilities.o HashTable.o
cflags = -Wall -g3
program = quic

$(program): $(objects)
	gcc $(objects) -o $(program)
	
quic.o: quic.c
	$(CC) $(cflags) -c quic.c

copiers.o: copiers.c
	$(CC) $(cflags) -c copiers.c

utilities.o: utilities.c
	$(CC) $(cflags) -c utilities.c

HashTable.o: HashTable.c
	$(CC) $(cflags) -c HashTable.c

run: $(program)
	./$(program) -l ../minefield ./clonefield -v -d

clean:
	rm -f $(program)
	rm -f $(objects)
