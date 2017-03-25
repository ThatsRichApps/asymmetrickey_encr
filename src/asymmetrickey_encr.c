/*
 * asymmetrickey_encr.c
 *
 *  Created on: Mar 20, 2017
 *      Author: Richard Humphrey
 *
 *  Last edited: 2017-03-24
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>

void print_help() {
	printf("Usage:\n");
	printf("./asymmetrickey_encr <key_file> <plaintext_file>\n");
}

void print_try_help() {
	printf("Try encrypt -h for help.\n");
}


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

	if (!key_file) {
		printf("Error: Provide the file containing the key pair.\n");
		print_try_help();
		exit(0);
	}

	optind++;

	plaintext_file = argv[optind];

	if (!plaintext_file) {
		printf("Error: Provide the file containing the plaintext.\n");
		print_try_help();
		exit(0);
	}
	optind++;

	printf ("keyfile: %s\n", key_file);
	printf ("textfile: %s\n", plaintext_file);

	// Read from the binary file into a buffer
	char *keybuffer;
	unsigned long kfileLen;

	//Open the file
	FILE *kfp = fopen(key_file,"r");
	if(!kfp) {
		perror ("Unable to read keyfile file");
		return 0;
	}

	//Get file length
	fseek(kfp, 0, SEEK_END);
	kfileLen=ftell(kfp);
	fseek(kfp, 0, SEEK_SET);

	//Allocate memory
	keybuffer=(char *)malloc(kfileLen+1);
	if (!keybuffer) {
		printf("Memory error!");
		fclose(kfp);
		return 0;
	}

	//Read file contents into buffer
	fread(keybuffer, kfileLen, 1, kfp);
	fclose(kfp);

	keybuffer[kfileLen] = '\0';

	printf ("Keys from file: |%s|\n", keybuffer);


	// Read from the binary file into a buffer
	char *buffer;
	unsigned long fileLen;

	//Open the file
	FILE *fp = fopen(plaintext_file,"r");
	if(!fp) {
		perror ("Unable to read plaintext file");
		return 0;
	}

	//Get file length
	fseek(fp, 0, SEEK_END);
	fileLen=ftell(fp);
	fseek(fp, 0, SEEK_SET);

	//Allocate memory
	buffer=(char *)malloc(fileLen+1);
	if (!buffer) {
		printf("Memory error!");
		fclose(fp);
		return 0;
	}

	//Read file contents into buffer
	fread(buffer, fileLen, 1, fp);
	fclose(fp);

	buffer[fileLen] = '\0';

	printf ("Plaintext from file: |%s|\n", buffer);

}
