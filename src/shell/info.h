#ifndef _shell_info_header_
#define _shell_info_header_

#include "type.h"

typedef void (*CommandFunction)(char* argv);

typedef struct {
    CommandFunction func;
    const char* name;
    const char* desc;
    void (*man)(void);
    int internal;
} Command;

const Command* getShellCommands(size_t* len);

#endif
