#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int main(int argc, char **argv)
{
	FILE *in, FILE *out;
	uint8_t c;

	if (argc < 3) {
		printf("[ ERROR ] Missing input and/or output arguments");
		exit(1);
	}

	in = fopen(argv[1], "r");
	if (in == NULL) {
		printf("[ ERROR ] Cannot open input file");
		exit(2);
	}

	out = fopen(argv[2], "w");
	if (out == NULL) {
		printf("[ ERROR ] Cannot open output file");
		exit(3);
	}
