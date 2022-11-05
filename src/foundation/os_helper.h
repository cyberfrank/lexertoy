#pragma once
#include "foundation/basic.h"

struct Allocator;

void *os_read_entire_file(const char *path, uint64_t *sz, struct Allocator *a);

uint64_t os_time_now();
double os_time_delta(uint64_t to, uint64_t from);

void *os_reserve(uint64_t size);
void os_release(void *mem);
void os_commit(void *mem, uint64_t size);
void os_decommit(void *mem, uint64_t size);
