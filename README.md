# 128-bit AES encryptor
This program implements AES 128-bit encryption in CBC mode for a variable number of rounds and iterations and a 16-byte key and plaintext

## Usage
	./aes <inputfile>

Iterations and rounds represented by decimal integers and the key and plaintext represented as a 32-character block of hex

The input file must be in the following form:

	<number of iterations>
	<number of rounds>
	<16-byte key>
	<16-byte plaintext>
