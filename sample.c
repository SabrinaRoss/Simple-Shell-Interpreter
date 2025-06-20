/**
 * You may use this code as part of your assignment.
 * 
 * If you choose to do so, you MUST indicate that
 * this is from "sample.c" starter code.
 * 
 * 
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

int main()
{
	const char* prompt = "shell> ";
	int bailout = 0;

	while (!bailout)
	{
		char* reply = readline(prompt);
		/* Note that readline strips away the final \n */

		if (!strcmp(reply, "bye"))
		{
			bailout = 1;
		}
		else
		{
			printf("\nYou said: %s\n\n", reply);
		}

		free(reply);
	}
	printf("Bye Bye\n");
	return 0;
}
