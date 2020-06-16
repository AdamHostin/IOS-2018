# prekladac
CC=gcc

# povinne flagy								
CFLAGS=-std=gnu99 -Wall -Wextra -Werror -pedantic


# $make
all: proj2

# *.o soubory
proj2.o: proj2.c
	$(CC) $(CFLAGS) -c proj2.c -o proj2.o

# binarka
proj2: proj2.o
	$(CC) $(CFLAGS) proj2.o -o proj2 -lpthread



# $make clean-o
clean-o:
	rm -f *.o

# $make clean-all
clean-all:
	rm -f proj2
	rm -f *.o
	rm -f proj2.zip
