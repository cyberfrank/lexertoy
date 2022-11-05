#include "allocator.h"
#include "foundation/os_helper.h"

static void *system_alloc(Allocator *a, void *old_ptr, uint64_t old_size, uint64_t new_size, 
    const char *file, uint32_t line)
{
    void *new_ptr = 0;
    if (new_size > 0) {
        new_ptr = realloc(old_ptr, new_size);
    } else {
        free(old_ptr);
    }
    return new_ptr;
}

static inline uint64_t align_size(uint64_t size, uint64_t align)
{
    return ((size + align - 1) / align) * align;
}

static void *fixed_vm_alloc(Allocator *a, void *old_ptr, uint64_t old_size, uint64_t new_size,
    const char *file, uint32_t line)
{
    old_size = align_size(old_size, 4096);
    new_size = align_size(new_size, 4096);

    if (new_size > 0 && new_size <= old_size)
        return old_ptr;

    uint64_t reserve_size = (uint64_t)a->user_data;
    if (new_size > reserve_size) {
        printf("Fixed virtual memory allocator out of memory! Wanted to allocate %zuB but there's only %zuB reserved\n",
            new_size, reserve_size);
        return 0;
    }

    void *new_ptr = 0;
    if (old_ptr == 0 && new_size > 0) {
        new_ptr = os_reserve(reserve_size);
        os_commit(new_ptr, new_size);
    } else if (new_size > 0) {
        new_ptr = old_ptr;
        os_commit((char *)new_ptr + old_size, new_size - old_size);
    } else if (new_size == 0) {
        os_release(old_ptr);
        // os_decommit(old_ptr, old_size);
        new_ptr = 0;
    }

    return new_ptr;
}

Allocator allocator_create_fixed_vm(uint64_t reserve_size)
{
    Allocator res = {
        .alloc_cb = fixed_vm_alloc,
        .user_data = (void *)reserve_size,
    };
    return res;
}

Allocator *system_allocator = &(Allocator) {
    .alloc_cb = system_alloc,
};
