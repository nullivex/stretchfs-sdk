//
// MemoryStruct memory buffer object
//

#include "../include/memory.h"
#include "../include/state.h"

size_t
InitMemory(void *userp) {
    auto MemoryStruct *mem = (MemoryStruct *)userp;
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
    mem->page = siSysInfo.dwPageSize;
#else
    mem->page = PAGESIZE;
#endif
    mem->memory = malloc(mem->page); /* will be grown as needed by the realloc in WriteMemoryCallback */
    mem->size = mem->page;           /* size is total alloc bytes */
    mem->ptr = 0;                    /* no data at this point */
    mem->memory[mem->ptr] = 0;       /* ensure null at first byte (cstr thus no need to memset entire page) */
#ifdef DEBUG
    printf("Allocated receive buffer (%u bytes)\n",(unsigned int)mem->size);
#endif
    return TRUE;
}