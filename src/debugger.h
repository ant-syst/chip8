#ifndef H_DEBUGGER
#define H_DEBUGGER

#include "chip8.h"

#include <stdint.h>

struct debugger;

struct debugger * dbg_alloc(struct chip8 * chip, int enable_debug);

void dbg_free(struct debugger ** dbg);

int dbg_call(struct debugger * dbg);

#endif
