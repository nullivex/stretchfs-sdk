//
// MemoryStruct memory buffer object
//

#include "../include/memory.hpp"
#include "../include/state.hpp"

size_t
MemPagesize() {
    size_t page = PAGESIZE;
#ifdef _MSC_VER
    SYSTEM_INFO siSysInfo;
    GetSystemInfo(&siSysInfo);
    // Display the contents of the SYSTEM_INFO structure.
    /*
    printf("Hardware information: \n");
    printf("  OEM ID: %u\n", siSysInfo.dwOemId);
    printf("  Number of processors: %u\n",siSysInfo.dwNumberOfProcessors);
    printf("  Page size: %u\n", siSysInfo.dwPageSize);
    printf("  Processor type: %u\n", siSysInfo.dwProcessorType);
    printf("  Minimum application address: %lx\n",siSysInfo.lpMinimumApplicationAddress);
    printf("  Maximum application address: %lx\n",siSysInfo.lpMaximumApplicationAddress);
    printf("  Active processor mask: %u\n",siSysInfo.dwActiveProcessorMask);
     */
    page = siSysInfo.dwPageSize;
#endif
    return page;
}

bool
InitMemory(void *userp) {
    auto *mem = (MemoryStruct *)userp;
    mem->page = MemPagesize();
    mem->memory = (char*)malloc(mem->page); /* will be grown as needed by the realloc in WriteMemoryCallback */
    mem->size = mem->page;           /* size is total alloc bytes */
    mem->ptr = 0;                    /* no data at this point */
    mem->memory[mem->ptr] = 0;       /* ensure null at first byte (cstr thus no need to memset entire page) */
#ifdef DEBUG
    printf("Allocated receive buffer (%u bytes)\n",(unsigned int)mem->size);
#endif
    return TRUE;
}

bool
ResetMemory(void *userp) {
    auto *mem = (MemoryStruct *)userp;
    if(0 != mem->ptr){
        mem->ptr = 0;
        mem->memory[mem->ptr] = 0;
#ifdef DEBUG
        printf("Reset receive buffer (%u bytes)\n",(unsigned int)mem->size);
#endif
    }
    return TRUE;
}