// This code is a template that can be adapted to one's needs
// Note that the code in this template will compile but will fail at runtime,
// unless one modifies it. To see a demo of how a proper application may
// look like, see the file main.c

#include "libcmdline.h"
#include <stdlib.h>

// First declare the command line options accepted by the application
Option op1 = {};
Option op2 = {};
Option op3 = {};
// More options as necessary ...

// Now add all the options to this array by reference (pointers)
Option* options[] = {
    &op1,
    &op2,
    &op3,
// Add more above here as more options are added but do not remove this NULL
    NULL
};

// Describe the application here
// Note that this is not mandatory and one may skip doing this if one wants to
Program desc = { "Optimiser", "0.0.1", "Copyright(C) John Doe 2021-25" };

int main(int argc, const char** argv) {
    // Register the details of the program here
    // If you choose not to use this feature, then comment the next line
    ProgramDetails(&desc);

    // Parse the options from the command line 
	int s = ParseOptions((Option**)&options, argc, argv);

    // If processing failed then take appropriate actions
	if (s != SUCCESS) {
		// code to deal with error here
        
        // Tell the executing environment that we failed
		return 1;
	}

    // Code to deal with the results of argument parsing here

    // Free the OptionArgs structure associated with each element of options
	FreeOptionArgs((Option**)&options);

    // Tell the executing environment that we succeeded
	return 0;
}

