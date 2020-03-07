#ifndef H_VM
#define H_VM

#include "io.h"
#include "debugger.h"

struct vm;

void vm_free(struct vm ** vm);

struct vm * vm_alloc(char const * its_logger_path);

int vm_run(struct vm * vm, struct chip8 * chip, struct io * io, struct debugger * dbg);

#endif
