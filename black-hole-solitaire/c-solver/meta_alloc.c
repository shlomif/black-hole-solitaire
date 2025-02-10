/*
 * This file is part of Freecell Solver. It is subject to the license terms in
 * the COPYING.txt file found in the top-level directory of this distribution
 * and at http://fc-solve.shlomifish.org/docs/distro/COPYING.html . No part of
 * Freecell Solver, including this file, may be copied, modified, propagated,
 * or distributed except according to the terms contained in the COPYING file.
 *
 * Copyright (c) 2000 Shlomi Fish
 */
// meta_alloc.c - the Freecell Solver compact allocator based on the
// meta-allocator concept that is used to collect the pages allocated by
// the standard allocator after it is destroyed and to recycle them.
#include <stdlib.h>
#include "meta_alloc.h"

#define ALLOCED_SIZE (FCS_IA_PACK_SIZE * 1024 - (256 + 128))

#define OLD_LIST_DATA(ptr) ((char *)(&(((char **)(ptr))[1])))
static inline char *meta_request_new_buffer(meta_allocator *const meta_alloc)
{
    char *const ret = meta_alloc->recycle_bin;
    if (ret)
    {
        meta_alloc->recycle_bin = FCS__COMPACT_ALLOC__OLD_LIST_NEXT(ret);
        return ret;
    }
    else
    {
        return malloc(ALLOCED_SIZE);
    }
}

bool bh_solve_compact_allocator_extend(compact_allocator *const allocator)
{
    char *const new_data = meta_request_new_buffer(allocator->meta);
    if (unlikely(!new_data))
    {
        return true;
    }

    FCS__COMPACT_ALLOC__OLD_LIST_NEXT(new_data) = allocator->old_list;
    allocator->old_list = new_data;

    allocator->ptr = allocator->rollback_ptr = OLD_LIST_DATA(new_data);
    allocator->max_ptr = new_data + ALLOCED_SIZE;
    return false;
}

void bh_solve_meta_compact_allocator_finish(meta_allocator *const meta_alloc)
{
    char *iter = meta_alloc->recycle_bin;
    char *iter_next = iter ? FCS__COMPACT_ALLOC__OLD_LIST_NEXT(iter) : NULL;
    for (; iter_next;
        iter = iter_next, iter_next = FCS__COMPACT_ALLOC__OLD_LIST_NEXT(iter))
    {
        free(iter);
    }
    free(iter);
    meta_alloc->recycle_bin = NULL;
}

#undef OLD_LIST_DATA
