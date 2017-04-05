
all: aes

aes: aes.c rcon.h sbox.h mixColumnTable.h
	gcc aes.c -O3 -std=c11 -o aes 

clean:
	rm aes