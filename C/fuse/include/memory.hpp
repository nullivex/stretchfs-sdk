#ifndef SFS_FUSE_MEMSTRUCT_H
#define SFS_FUSE_MEMSTRUCT_H

#include <cstdarg>

#define PAGESIZE 4096

typedef struct {
    char *memory;
    size_t page;
    size_t size;
    size_t ptr;
    struct json_object *parsed;
} MemoryStruct;

bool InitMemory(void *);
bool ResetMemory(void *);

#endif //SFS_FUSE_MEMSTRUCT_H