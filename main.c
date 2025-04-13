#include "libcmdline.h"
#include <stdlib.h>

Option options[] = {
	{"o", "optimise", "Specify the degree of optimisation", "n", NULL,
		NULL, 1, OPTION_REQUIRED },
	{"f", "file", "Specify the input file", "s", NULL, NULL, 
		1, OPTION_REQUIRED }
};

Program desc = { "Optimiser", "0.0.1", "Copyright(C) John Doe 2021-25" };
int main(int argc, char** argv) {
	ProgramDetails(&desc);
	RegisterOptions(options, 2);
	if (ParseOptions(argc, argv) < 0)
		return 1;

	return 0;
}

