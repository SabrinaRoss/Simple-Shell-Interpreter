/**
 * You may use this code or as part of your assignment.
 * 
 * If you choose to do so, you MUST indicate that
 * this is from "args.c" starter code.
 * 
 * 
 */
#include <stdio.h>

int main(int argc, char* argv[])
{
	int i;

	if (argc < 2)
	{
		fprintf(stderr, "Usage: Would like some command-line arguments\n");
	}
	else
	{
		printf("------------------------------------------------\n");
		for (i = 0 ; i < argc; ++i)
		{
			fprintf(stderr, "Argument No %d: %s\n", i, argv[i]);
		}
		printf("------------------------------------------------\n");
	}
	return 0;
}
