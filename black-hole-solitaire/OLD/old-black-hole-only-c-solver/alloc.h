/* Copyright (c) 2000 Shlomi Fish
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */
/*
 * alloc.h - the Freecell Solver compact allocator. Used to allocate
 * columns and other small allocations of a short size. Is able to revert the
 * last allocation.
 */
#ifndef FC_SOLVE__ALLOC_H
#define FC_SOLVE__ALLOC_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "inline.h"

typedef struct
{
    char * * packs;
    int num_packs;
    char * max_ptr;
    char * ptr;
    char * rollback_ptr;
} bhs_compact_allocator_t;

extern void
    bh_solve_compact_allocator_init(bhs_compact_allocator_t * allocator);

extern void bh_solve_compact_allocator_extend(
    bhs_compact_allocator_t * allocator
        );

static GCC_INLINE void * fcs_compact_alloc_ptr(bhs_compact_allocator_t * allocator, int how_much)
{
    /* Round ptr to the next pointer boundary */
    how_much +=
        (
         (sizeof(char *)-((how_much)&(sizeof(char *)-1)))&(sizeof(char*)-1)
        );

    if (allocator->max_ptr - allocator->ptr < how_much)
    {
        bh_solve_compact_allocator_extend(allocator);
    }
    else
    {
        allocator->rollback_ptr = allocator->ptr;
    }
    allocator->ptr += how_much;

    return allocator->rollback_ptr;
}

#define fcs_compact_alloc_release(allocator) \
{    \
    (allocator)->ptr = (allocator)->rollback_ptr; \
}

extern void bh_solve_compact_allocator_finish(bhs_compact_allocator_t * allocator);

#ifdef __cplusplus
};
#endif

#endif
