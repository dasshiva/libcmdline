#ifndef __LIBCMDLINE_H__
#define __LIBCMDLINE_H__

#ifdef __cplusplus
extern "C" {
namespace cmdline {
#endif

#include <stdint.h>

typedef struct {
    union {
        char*    String;
        int64_t  Number;
    };
    uint32_t Type;
} OptionArgs;

#define TYPE_STRING (1U << 0)
#define TYPE_NUMBER (1U << 1)

typedef int (*OptProcess)(OptionArgs*, uint32_t);

// the option format basically looks like this
// opt-format = opt-spec { '-' opt-spec }
// opt-spec = 's' | 'n'
// s represents a string and n represents a number
// Some valid examples include: "s-n-n" or "n-s" but not "ns-n"
typedef struct {
    const char* ShortOption;
    const char* LongOption;
    const char* Help;
    const char* Fmt;
    OptProcess  Func;
    OptionArgs* Args;
    uint32_t    NArgs;
    uint32_t    Flags;
} Option;

typedef struct {
    const char* Name;
    const char* Version;
    const char* Copyright;
} Program;

#define OPTION_REQUIRED (1U << 0)
#define OPTION_DEFAULT  (1U << 1)

int ProgramDetails(Program* prog);
int RegisterOptions(Option* options, uint32_t len);
int ParseOptions(int argc, char** argv);
int FreeOptions(Option* options, uint32_t len);

#define ARG_NULL (-1)
#define UNEXPECTED_FORMAT_ARG (-2)
#define INVALID_OPTION_ARGS (-3)
#define DUPLICATE_DEFAULT_OPTION (-4)
#define INVALID_DEFAULT_OPTION_ARGS (-5)

#define SUCCESS  (1)
#define INVALID_OPTION_INDEX(x) (((x) > SUCCESS) ? ((x) - SUCCESS - 1) : -1)

#ifdef __cplusplus
}
}
#endif

#endif
