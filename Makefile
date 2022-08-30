all: fat

fat: fat.c
	gcc -Wall -o fat fat.c

clean: 
	rm -fr *~ fat
