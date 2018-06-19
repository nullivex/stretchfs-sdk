#ifndef SFS_FUSE_MEMSTRUCT_H
#define SFS_FUSE_MEMSTRUCT_H

#include <stdarg.h>

#define PAGESIZE 4096
struct MemoryStruct {
    char *memory;
    size_t page;
    size_t size;
    size_t ptr;
    struct json_object *parsed;
};

void InitMemory(void *);

#endif //SFS_FUSE_MEMSTRUCT_H