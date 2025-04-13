#include "libcmdline.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define NO_WORK                 (1U << 0)
#define REQUIRED_OPTION_PRESENT (1U << 1)
#define DEFAULT_OPTION_PRESENT  (1u << 2)

static Program* program    = NULL;
static Option*  opts       = NULL;
static uint32_t optlen     = 0U;
static uint32_t stateflags = 0U;
static Option*  dopt       = NULL;
static int init = 0;

// Internal values of Option->flags
#define NO_HELP_OPTION    (1U << 4)
#define HAVE_SHORT_OPTION (1U << 6)
#define HAVE_LONG_OPTION  (1U << 7)


static int ParseSchema(Option opt, const char* fmt) {
    if (!fmt)
        return SUCCESS;

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

    if (n != opt.NArgs)
        return -1;

    // If opt.args is initialised these are the default values
    // for the option's arguments and we should not touch them
    if (!opt.Args)
        opt.Args = malloc(sizeof(OptionArgs) * n);

    if (!opt.Args)
        return -1;

    return SUCCESS;
}

int ProgramDetails(Program* prog) {
    if (!prog)
        return ARG_NULL;

    program = prog;
    return SUCCESS;
}

int RegisterOptions(Option* options, uint32_t len) {
    if (!options && len) // No options but len > 0
        return ARG_NULL;

    opts = options;
    optlen = len;

    if (!optlen)
        stateflags |= NO_WORK;

    for (uint32_t opt = 0; opt < len; opt++) {
        Option option = options[opt];
        if (option.ShortOption)
            option.Flags |= HAVE_SHORT_OPTION;

        if (option.LongOption)
            option.Flags |= HAVE_LONG_OPTION;

        if (!(option.Flags & HAVE_SHORT_OPTION) && 
                !(option.Flags & HAVE_LONG_OPTION)) 
            return (opt + SUCCESS + 1); // + 1 ensures we don't return SUCCESS

        if (!option.Help)
            option.Flags |= NO_HELP_OPTION;

        if ((!option.NArgs && option.Fmt) || (option.NArgs && !option.Fmt))
            return UNEXPECTED_FORMAT_ARG;

        if (ParseSchema(option, option.Fmt) < 0)
            return (opt + SUCCESS + 1);

        if (option.Flags & OPTION_REQUIRED) 
            stateflags |= REQUIRED_OPTION_PRESENT;

        if (option.Flags & OPTION_DEFAULT) {
            if (stateflags & DEFAULT_OPTION_PRESENT)
                return DUPLICATE_DEFAULT_OPTION;
            stateflags |= DEFAULT_OPTION_PRESENT;
            dopt = &option;
        }
    }

    return SUCCESS;
}

static int ParseShortOption(int*, int, char**, char*, int);
static int ParseLongOption(int*, int, char**, char*, int);
static int ParseDefaultOptionArgs(int*, int, char**, char*);

int ParseOptions(int argc, char** argv) {
    if (argc == 1) {
        if ((stateflags & NO_WORK) || 
           !((stateflags & REQUIRED_OPTION_PRESENT) &&
               (stateflags & DEFAULT_OPTION_PRESENT)))
            return SUCCESS;

        // Default option itself has default arguments
        if (dopt->Args)
            return SUCCESS;
    }

    // Save the program name
    const char* progname = argv[0];

    // Remove the program name from the argument list
    argv++; 
    argc--;

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
    for (int i = 0; i < argc;) {
        char* sptr = argv[i];
        int len = strlen(sptr);
        i++;

        if (len > 2) {
            if (sptr[0] == '-') 
                if (ParseShortOption(&i, argc, argv, sptr, len - 1) < 0)
                    return INVALID_OPTION;

            if (ParseDefaultOptionArgs(&i, argc, argv, sptr) < 0)
                return INVALID_DEFAULT_OPTION_ARGS;
        }

        else if (len > 3) {
            if (sptr[0] == '-' && sptr[1] == '-')
                if (ParseLongOption(&i, argc, argv, sptr, len - 2) < 0)
                    return INVALID_OPTION;

            if (ParseDefaultOptionArgs(&i, argc, argv, sptr) < 0)
                return INVALID_DEFAULT_OPTION_ARGS;
        }

        else { /* All cases are already covered */ }

    }

    return SUCCESS;
}

static Option* FindShortOpt(const char* name, int len) {
    for (uint32_t n = 0; n < optlen; n++) {
        if (strncmp(opts[n].ShortOption, name, len) == 0)
            return &opts[n];
    }

    return NULL;
}

static Option* FindLongOpt(const char* name, int len) {
    for (uint32_t n = 0; n < optlen; n++) {
        if (strncmp(opts[n].LongOption, name, len) == 0)
            return &opts[n];
    }

    return NULL;
}


static int ParseArgs(Option*, int*, int, char**);

static int ParseShortOption(int* idx, int argc, char** argv, 
        char* opt, int len) {
    // If we came here, len > 2
    opt++;
    Option* option = FindShortOpt(opt, len);
    if (!option)
        return UNKNOWN_SHORT_OPTION;

    option->Flags |= OPTION_PRESENT;

    *idx++;
    if (!option->NArgs)
        return SUCCESS;

    return ParseArgs(option, idx, argc, argv);
}

static int ParseLongOption(int* idx, int argc, char** argv, 
        char* opt, int len) {
    // If we came here, len > 3
    opt += 2;
    Option* option = FindLongOpt(opt, len);
    if (!option)
        return UNKNOWN_LONG_OPTION;

    option->Flags |= OPTION_PRESENT;
    if (!option->NArgs)
        return SUCCESS;

    *idx++;
    return ParseArgs(option, idx, argc, argv); 
}

static int ParseDefaultOptionArgs(int* idx, int argc, char** argv, 
        char* opt) {
	dopt |= OPTION_PRESENT;
    return ParseArgs(dopt, idx, argc, argv);
}

static int ParseArgs(Option* opt, int* idx, int argc, char** argv) {
	char* schema = opt->Fmt;

    return SUCCESS;
}
