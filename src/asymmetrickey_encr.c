/*
 * asymmetrickey_encr.c
 *
 *  Created on: Mar 20, 2017
 *      Author: Richard Humphrey
 *
 *  Last edited: 2017-04-05
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <gmp.h>


struct PublicKey {
	mpz_t e;
	mpz_t n;
};

void print_help();
void print_try_help();
struct PublicKey *read_keyfile (char *keyfile);
int power(int x, unsigned int y, int p);

int main (int argc, char **argv) {

	char *plaintext_file;
	char *key_file;
	int c;

	if (argc == 1) {
			printf("Error: Provide file containing key and plaintext file.\n");
			print_try_help();
			exit(0);
	}

	while ((c = getopt (argc, argv, "h")) != -1) {
		// Option argument
		switch (c) {
		case 'h':
			print_help();
			exit(1);
		default:
			break;
		}
	}

	key_file = argv[optind];

	struct PublicKey *public_key = read_keyfile(key_file);

	gmp_printf ("key e = %Zd\n", public_key->e);
	gmp_printf ("key n = %Zd\n", public_key->n);

	long unsigned int n_size = mpz_sizeinbase(public_key->n, 2);

	int blocksize = n_size / 8;

	printf ("blocksize = %i\n", blocksize);

	optind++;

	plaintext_file = argv[optind];

	if (!plaintext_file) {
		printf("Error: Provide the file containing the plaintext.\n");
		print_try_help();
		exit(0);
	}
	optind++;

	// Read from the file into a buffer
	unsigned long fileLen;

	//Open the file as a binary file
	FILE *fp = fopen(plaintext_file,"r");
	if(!fp) {
		perror ("Unable to read plaintext file.\n");
		return 0;
	}

	//Get file length
	fseek(fp, 0, SEEK_END);
	fileLen=ftell(fp);
	fseek(fp, 0, SEEK_SET);

	//Read file contents into buffer
	printf ("file length is %lu chars\n", fileLen);

	char *block;
	//Allocate memory
	block= (char *)malloc(blocksize+1);
	if (!block) {
		printf("Unable to allocate memory for block\n");
		fclose(fp);
		return 0;
	}

	int nbytes;

	// write to output file
	char *output_filename = "Richard-Humphrey.ciphertext";
	FILE *wp;
	wp = fopen(output_filename,"w");

	char delimiter = '\0';

	// read one block at a time, encrypt and write as we go
	while ( (nbytes = fread(block, 1, blocksize, fp)) > 0) {

		mpz_t buffer_int;
		mpz_init_set_ui(buffer_int, 0);

		mpz_t encrypted;
		mpz_init_set_ui(encrypted, 0);

		block[nbytes] = '\0';

		printf ("blocksize = %i\n", nbytes);

		// this doesn't work
		//mpz_set_str(buffer_int, block, 62);

		// loop through block to convert from ascii to int
		for (int i = 0; i < nbytes; i++) {

			//printf ("bytes - %i, i - %i, power - %i, block %c\n", nbytes, i, ((nbytes-i-1)*8), block[i]);

			mpz_t value;
			mpz_init(value);
			mpz_ui_pow_ui(value, 2, (nbytes-i-1)*8);
			mpz_mul_ui(value, value, block[i]);

			mpz_add(buffer_int, buffer_int, value);

			//gmp_printf ("char - %c - int - %Zd\n", block[i], value);
			mpz_clear(value);

		}


		printf ("Plain text from file: |%s|\n", block);
		gmp_printf ("Buffer Int: |%Zd|\n", buffer_int);

		mpz_powm(encrypted, buffer_int, public_key->e, public_key->n);
		gmp_printf ("Ciphertext: |%Zd|\n", encrypted);


		mpz_out_str(wp, 10, encrypted);

		fputc (delimiter, wp);

		mpz_clear(buffer_int);
		mpz_clear(encrypted);


	}

	fclose(fp);
	fclose(wp);

	return 0;

}

// Functions:

void print_help() {
	printf("Usage:\n");
	printf("./asymmetrickey_encr <key_file> <plaintext_file>\n");
}

void print_try_help() {
	printf("Try encrypt -h for help.\n");
}

struct PublicKey *read_keyfile (char *key_file){
	// Reads the keyfile and created the Publickey struct

	if (!key_file) {
		printf("Error: Provide the file containing the key pair.\n");
		print_try_help();
		exit(0);
	}

	// Read from the binary file into a buffer
	char *keybuffer;
	unsigned long keyfileLen;
	struct PublicKey *key = (struct PublicKey *) malloc (sizeof(struct PublicKey));

	char *e_str;
	char *n_str;
	int e_length = 0;
	int n_length = 0;

	mpz_init(key->e);
	mpz_init(key->n);

	//Open the file
	FILE *kfp = fopen(key_file,"r");
	if(!kfp) {
		printf ("Unable to read file %s", key_file);
		exit(0);
	}

	//Get file length
	fseek(kfp, 0, SEEK_END);
	keyfileLen=ftell(kfp);
	fseek(kfp, 0, SEEK_SET);

	//Allocate memory
	keybuffer=(char *)malloc(keyfileLen+1);
	if (!keybuffer) {
		printf("Unable to allocate memory to read keyfile");
		fclose(kfp);
		exit(0);
	}

	for (int i = 0; i < keyfileLen; i++) {
		int c = fgetc(kfp);

		if (!feof(kfp)) {
			if (c == '\n') {
				keybuffer[i] = '\0';
				break;
			} else {
				keybuffer[i] = c;
				// remember the location of the comma
				if (c == ',') {
					e_length = i;
				}
			}
		} else {
			keybuffer[i] = '\0';
			break;
		}

	}

	if (e_length == 0) {
		printf ("Error, file should be comma delimited\n");
		exit(0);
	}

	int keylength = strlen(keybuffer);

	printf ("File buffer is: |%s|\n", keybuffer);

	e_str = malloc(sizeof(char) * e_length + 1);

	n_length = (keylength - e_length - 2);

	n_str = malloc(sizeof (char) * n_length + 1);

	printf ("size of keys are e = %i, n = %i\n", e_length, n_length);

	strncpy (e_str, keybuffer, e_length);
	strncpy (n_str, &keybuffer[e_length+2], n_length);

	//printf ("e = %s\n", e_str);
	//printf ("n = %s\n", n_str);

	mpz_set_str(key->e, e_str, 10);
	mpz_set_str(key->n, n_str, 10);


	free(e_str);
	free(n_str);
	free(keybuffer);

	return (key);

}


//* Iterative Function to calculate (x^y)%p in O(log y) */
int power(int x, unsigned int y, int p)
{
    int res = 1;      // Initialize result

    x = x % p;  // Update x if it is more than or
                // equal to p

    while (y > 0)
    {
        // If y is odd, multiply x with result
        if (y & 1)
            res = (res*x) % p;

        // y must be even now
        y = y>>1; // y = y/2
        x = (x*x) % p;
    }
    return res;
}

