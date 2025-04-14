#include "libcmdline.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define OPTION_HELP            ((Option*)((void*)-1))
#define NO_WORK                 (1U << 0)
#define REQUIRED_OPTION_PRESENT (1U << 1)
#define DEFAULT_OPTION_PRESENT  (1u << 2)

static Program* program    = NULL;
// Internal values of Option->flags
#define OPTION_DONE         (1U << 3)
#define SELF_ALLOCATED_ARGS (1U << 4)

static int ParseSchema(Option* opt, const char* fmt) {
    if (!fmt)
        return CMDLINE_SUCCESS;

    int len = strlen(fmt);
    if (!(len % 2)) // There are always an odd no: of entries in fmt
        return -1;

    uint32_t n = 0;
    for (int i = 0; i < len; i++) {
        if (!(i % 2)) { // must be opt-spec
            switch (fmt[i]) {
                case 'n':
                case 's': n++; break;
                default: return -1;
            }
        }
        else {
            if (fmt[i] != '-')
                return -1;
        }
    }

    if (n != opt->NArgs)
        return -1;

    return CMDLINE_SUCCESS;
}

int ProgramDetails(Program* prog) {
    if (!prog)
        return ARG_NULL;

    program = prog;
    return CMDLINE_SUCCESS;
}

static Option* FindShortOpt(Option** opts, const uint32_t oplen, 
        const char* name) {

    for (uint32_t n = 0; n < oplen; n++) {
        if (!opts[n]->ShortOption) 
            n++;

        if (strcmp(opts[n]->ShortOption, name) == 0)
            return opts[n];
    }

    if (strcmp(name, "h") == 0)
        return OPTION_HELP;

    return NULL;
}

static Option* FindLongOpt(Option** opts, const uint32_t oplen, 
        const char* name) {

    for (uint32_t n = 0; n < oplen; n++) {
        if (!opts[n]->LongOption) 
            continue;
        

        if (strcmp(opts[n]->LongOption, name) == 0)
            return opts[n];
    }

    if (strcmp(name, "h") == 0)
        return OPTION_HELP;

    return NULL;
}


static int ParseArgs(Option*, int*, int, const char**);

static int ParseShortOption(Option** opts, int* idx, const int argc, 
        const char** argv, const char* opt, const uint32_t oplen) {
    // If we came here, len > 2
    const char* opname = (opt + 1);
	(*idx)++;

    Option* option = FindShortOpt(opts, oplen, opname);
    if (!option)
        return UNKNOWN_SHORT_OPTION;

    if (option == OPTION_HELP) {
        GenerateHelp(argv[0], opts, oplen);
        return CMDLINE_SUCCESS;
    }

    if (option->Flags & OPTION_DONE)
        return DUPLICATE_OPTION;

    option->Flags |= OPTION_PRESENT;

    if (!option->NArgs)
        return CMDLINE_SUCCESS;
    return ParseArgs(option, idx, argc, argv);
}

static int ParseLongOption(Option** opts, int* idx, const int argc, 
        const char** argv, const char* opt, const uint32_t oplen) {
    // If we came here, len > 3
    const char* opname = (opt + 2);
	(*idx)++;
    Option* option = FindLongOpt(opts, oplen, opname);
    if (!option)
        return UNKNOWN_LONG_OPTION;

     if (option == OPTION_HELP) {
        GenerateHelp(argv[0], opts, oplen);
        return CMDLINE_SUCCESS;
    }

     if (option->Flags & OPTION_DONE)
        return DUPLICATE_OPTION;

    option->Flags |= OPTION_PRESENT;
    if (!option->NArgs)
        return CMDLINE_SUCCESS;

    return ParseArgs(option, idx, argc, argv); 
}

static int ParseDefaultOptionArgs(Option* defopt, int* idx, int argc, 
        const char** argv, const char* opt) {
    if (!defopt)
       return NO_DEFAULT_OPTION;

    if (defopt->Flags & OPTION_DONE)
        return DUPLICATE_OPTION;

	int s = ParseArgs(defopt, idx, argc, argv);
	defopt->Flags |= OPTION_PRESENT;
	return s;
}

static int ParseArgs(Option* opt, int* idx, int argc, const char** argv) {
    char* schema = opt->Fmt;
    uint32_t opt_idx = 0;

	// If opt.args is initialised these are the default values
    // for the option's arguments and we should not touch them
    if (!opt->Args) {
		opt->Flags |= SELF_ALLOCATED_ARGS;
        opt->Args = malloc(sizeof(OptionArgs) * opt->NArgs);
	}

    while (*schema) {
        if (*idx >= argc)
            return INSUFFICIENT_OPTION_ARGS;

        if (*schema == '-') {
            schema++;
            continue;
        }

        else if (*schema == 's') {
            opt->Args[opt_idx].String = argv[*idx];
            opt->Args[opt_idx].Type = TYPE_STRING;
            opt_idx++;
            (*idx)++;
        }

        else {
            char* endptr = "";
            int64_t n = strtoll(argv[*idx], &endptr, 0);
            if (*endptr)
                return INVALID_INTEGER_LITERAL;
            opt->Args[opt_idx].Number = n;
            opt->Args[opt_idx].Type = TYPE_NUMBER;
            opt_idx++;
            (*idx)++;
        }

		schema++;
        
    }

    if (opt->Func)
        if (opt->Func(opt->Args, opt->NArgs) < 0)
            return USER_FUNC_ERROR;

    opt->Flags |= OPTION_DONE;
    return CMDLINE_SUCCESS;
}

void GenerateHelp(const char* progname, Option** opts, const uint32_t oplen) {
    if (program)
        fprintf(stdout, "%s %s\n%s\n", program->Name, program->Version,
                program->Copyright);
    fprintf(stdout, "%s [OPTIONS]\n", progname);

    fprintf(stdout, "Available options: \n");
    for (uint32_t opt = 0; opt < oplen; opt++) {
        Option* option = opts[opt];
        if (option->ShortOption)
            fprintf(stdout, "-%s", option->ShortOption);

        if (option->ShortOption && option->LongOption)
            fprintf(stdout, "/");

        if (option->LongOption)
            fprintf(stdout, "--%s ", option->LongOption);

        if (option->Help) {
            if (!option->LongOption)
                fprintf(stdout, "\t\t- %s", option->Help);
            else
                fprintf(stdout, "\t- %s", option->Help);
        }

        fprintf(stdout, "\n");
    }
}

int ParseOptions(Option** opts, const int argc, const char** argv) {
    uint32_t oplen = 0, state = 0;
    Option** tmp = opts;
    Option* defopt = NULL;

    while (*tmp) {
        tmp++;
        oplen++;
    }

    if (!oplen)
        state |= NO_WORK;

    tmp = opts;
    uint32_t opt = 0;
    while (*tmp) {
        Option* option = *tmp;
        if (!(option->ShortOption) && 
                !(option->LongOption)) 
            return (opt + CMDLINE_SUCCESS + 1); // + 1 ensures we don't return SUCCESS

        if ((!option->NArgs && option->Fmt) || (option->NArgs && !option->Fmt))
            return UNEXPECTED_FORMAT_ARG;

        if (ParseSchema(option, option->Fmt) < 0)
            return (opt + CMDLINE_SUCCESS + 1);

        if (option->Flags & OPTION_REQUIRED) 
            state |= REQUIRED_OPTION_PRESENT;

        if (option->Flags & OPTION_DEFAULT) {
            if (state & DEFAULT_OPTION_PRESENT)
                return DUPLICATE_DEFAULT_OPTION;
            state |= DEFAULT_OPTION_PRESENT;
            defopt = option;
        }

        opt++;
        tmp++;
    }

    if (argc == 1) {
        if ((state & NO_WORK) || 
           !((state& REQUIRED_OPTION_PRESENT) &&
               (state & DEFAULT_OPTION_PRESENT)))
            return CMDLINE_SUCCESS;

        // Default option itself has default arguments
        if (defopt) 
			if (defopt->Args)
				return CMDLINE_SUCCESS;
    }

    // Save the program name
    const char* progname = argv[0]; 

    // All options look like this
    // Option = LONG_OPT | SHORT_OPT
    // LONG_OPT = "--" LONG_OPT_TEXT ARGS
    // SHORT_OPT = '-' SHORT_OPT_TEXT ARGS
    // LONG_OPT_TEXT = <pseudo symbol representing Option->LongOption >
    // SHORT_OPT_TEXT = <pseudo symbol representing Option->ShortOption > 
    // ARGS = ARG { (" ")+ ARG }
    // ARG = string | number
    // string = <pseudo symbol that represents all sequences of characters>
    // number = <pseudo symbol that represents all sequences of digits>
    for (int i = 1; i < argc;) {
        const char* sptr = argv[i];
        int len = strlen(sptr);

		if (len >= 3) {
            if (sptr[0] == '-') { 
                if (sptr[1] == '-') {
                    if (ParseLongOption(opts, &i, argc, argv, sptr, oplen) < 0)
                        return INVALID_OPTION;
                }

                else {
                    if (ParseShortOption(opts, &i, argc, argv, sptr, oplen) < 0)
                        return INVALID_OPTION;
                }
			}

			else if (ParseDefaultOptionArgs(defopt, &i, argc, argv, sptr) < 0) 
                return INVALID_DEFAULT_OPTION_ARGS;
			
        }

		else if (len == 2) {
            if (sptr[0] == '-') {
                if (ParseShortOption(opts, &i, argc, argv, sptr, oplen) < 0)
                    return INVALID_OPTION;
			}

			else if (ParseDefaultOptionArgs(defopt, &i, argc, argv, sptr) < 0) 
                return INVALID_DEFAULT_OPTION_ARGS;
			
        }


        else {
            if (ParseDefaultOptionArgs(defopt, &i, argc, argv, sptr) < 0) 
                return INVALID_DEFAULT_OPTION_ARGS;
        }

    }

	for (uint32_t id = 0; id < oplen; id++) {
		Option* opt = opts[id];
		if ((opt->Flags & OPTION_REQUIRED) && !(opt->Flags & OPTION_PRESENT)) {
			if (opt->ShortOption) 
				fprintf(stdout, "Required option -%s not found\n", opt->ShortOption);
			else
				fprintf(stdout, "Required option --%s not found\n", opt->LongOption);
			return REQUIRED_OPTION_MISSING;

		}
	}

    return CMDLINE_SUCCESS;
}

void FreeOptionArgs(Option** args) {
	while (*args) {
		Option* opt = *args;
		if (opt->Flags & SELF_ALLOCATED_ARGS) 
			free(opt->Args);
		args++;
	}
}

