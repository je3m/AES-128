
all: aes

AES: aes.c
	gcc aes.c -o aes

clean:
	rm aes.o aes *~