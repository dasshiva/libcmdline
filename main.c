#include "libcmdline.h"
#include <stdlib.h>

Option optimise = { NULL, "optimise", "Specify the degree of optimisation", 
    "n", NULL, NULL, 1, OPTION_REQUIRED };
Option file = { "f", "file", "Specify the input file", "s", NULL, NULL, 
		1, OPTION_REQUIRED | OPTION_DEFAULT };

Option* options[] = {
    &optimise,
    &file,
    NULL
};

Program desc = { "Optimiser", "0.0.1", "Copyright(C) John Doe 2021-25" };
int main(int argc, const char** argv) {
    ProgramDetails(&desc);
	if (ParseOptions((Option**)&options, argc, argv) < 0)
		return 1;
	return 0;
}

