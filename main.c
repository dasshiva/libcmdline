#include "libcmdline.h"
#include <stdlib.h>
#include <stdio.h>

Option optimise = { NULL, "optimise", "Specify the degree of optimisation", 
    "n", NULL, NULL, 1, OPTION_REQUIRED };
Option file = { "f", NULL, "Specify the input file", "s", NULL, NULL, 
		1, OPTION_REQUIRED | OPTION_DEFAULT };

Option* options[] = {
    &optimise,
    &file,
    NULL
};

Program desc = { "Optimiser", "0.0.1", "Copyright(C) John Doe 2021-25" };
int main(int argc, const char** argv) {
    ProgramDetails(&desc);
	int s = ParseOptions((Option**)&options, argc, argv);
	if (s != SUCCESS) {
		printf("Result = %d\n", s);
		return 1;
	}

	printf("File = %s\n", file.Args->String);
	printf("Optimisation = %ld\n", optimise.Args->Number);
	FreeOptionArgs((Option**)&options);
	return 0;
}

