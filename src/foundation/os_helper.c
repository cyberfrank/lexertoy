#include "os_helper.h"
#include "foundation/allocator.h"

#include <windows.h>

void *os_read_entire_file(const char *path, uint64_t *sz, Allocator *a)
{
    uint64_t size = 0;
    FILE *f = fopen(path, "rb");
    if (f == NULL)
        return 0;
    
    fseek(f, 0L, SEEK_END);
    size = ftell(f);
    fseek(f, 0L, SEEK_SET);

    void *data = c_alloc(a, size);
    uint64_t actual_size = fread(data, 1, size, f);
    if (actual_size != size) {
        printf("Failed to read entire file!\n");
        c_free(a, data, size);
        return 0;
    }
    *sz = size;
    fclose(f);
    return data;
}

uint64_t os_time_now()
{
    uint64_t now;
    LARGE_INTEGER qpc;
    QueryPerformanceCounter(&qpc);
    now = (uint64_t)qpc.QuadPart;
    return now;
}

double os_time_delta(uint64_t to, uint64_t from)
{
    double result = 0.0;
    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);
    result = (to - from) * (1.0 / freq.QuadPart);
    return result;
}

void *os_reserve(uint64_t size)
{
    void *mem = VirtualAlloc(0, size, MEM_RESERVE, PAGE_NOACCESS);
    return mem;
}

void os_release(void *mem)
{
    VirtualFree(mem, 0, MEM_RELEASE);
}

void os_commit(void *mem, uint64_t size)
{
    VirtualAlloc(mem, size, MEM_COMMIT, PAGE_READWRITE);
}

void os_decommit(void *mem, uint64_t size)
{
    VirtualFree(mem, size, MEM_DECOMMIT);
}
