#ifndef __LIBCMDLINE_H__
#define __LIBCMDLINE_H__

#ifdef __cplusplus
extern "C" {
namespace cmdline {
#endif

#include <stdint.h>

// Represents the arguments given to a function
typedef struct {
    union {
        const char* String; // String argument to option
        int64_t     Number; // Number argument to option
    };
    uint32_t Type;
} OptionArgs;

// Values for OptionArgs.Type
#define TYPE_STRING (1U << 0) // OptionArgs.String is the active field
#define TYPE_NUMBER (1U << 1) // OptionArgs.Number is the active field

typedef int (*OptProcess)(OptionArgs*, uint32_t);

// the format of Option.Fmt basically looks like this
// opt-format = opt-spec { '-' opt-spec }
// opt-spec = 's' | 'n'
// s represents a string and n represents a number
// Some valid examples include: "s-n-n" or "n-s" but not "ns-n"
//
// Represents a single option on the command line
typedef struct {
    const char* ShortOption; // Shorter form of option (prefixed by '-')
    const char* LongOption; // Longer form of option (prefixed by "--")
    const char* Help; // Help text describing what the option does
    char*       Fmt; // As described above
    OptProcess  Func; // Function to call after parsing the option
    OptionArgs* Args; // Pointer to the argumengs given to the option
    uint32_t    NArgs; // Number of arguments taken by the options
    uint32_t    Flags; // Flags describing the option3
} Option;

typedef struct {
    const char* Name; // Name of the program
    const char* Version; // Version of the program
    const char* Copyright; // Copyright associated with program
} Program;

// Values for Option.Flags
#define OPTION_REQUIRED (1U << 0) // Option must be present
// Default option. All unused arguments go to this option
#define OPTION_DEFAULT  (1U << 1) 
// This option was specified on the command line
// Do not set this flag yourself if you are a user of the library
#define OPTION_PRESENT  (1U << 2)

// Register the details of the program
int ProgramDetails(Program* prog);

// Free Option.Args
int FreeOptionArgs(Option* options, uint32_t len);

// Parse command line options from argv having argc command line args
int ParseOptions(Option** opts, const int argc, const char** argv);

// Generate a help text from all the non-null values of Option.Help from
// the options present in opts
void GenerateHelp(const char* progname, Option** opts, const uint32_t oplen);

#define ARG_NULL (-1)
#define UNEXPECTED_FORMAT_ARG (-2)
#define INVALID_OPTION (-3)
#define DUPLICATE_DEFAULT_OPTION (-4)
#define INVALID_DEFAULT_OPTION_ARGS (-5)
#define UNKNOWN_SHORT_OPTION (-6)
#define UNKNOWN_LONG_OPTION (-7)
#define INSUFFICIENT_OPTION_ARGS (-8)
#define INVALID_INTEGER_LITERAL (-9)
#define USER_FUNC_ERROR (-10)
#define DUPLICATE_OPTION (-11)
#define REQUIRED_OPTION_MISSING (-12)

#define SUCCESS  (1)
#define INVALID_OPTION_INDEX(x) (((x) > SUCCESS) ? ((x) - SUCCESS - 1) : -1)

#ifdef __cplusplus
}
}
#endif

#endif
