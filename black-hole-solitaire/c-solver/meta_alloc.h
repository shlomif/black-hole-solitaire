/*
 * This file is part of Freecell Solver. It is subject to the license terms in
 * the COPYING.txt file found in the top-level directory of this distribution
 * and at http://fc-solve.shlomifish.org/docs/distro/COPYING.html . No part of
 * Freecell Solver, including this file, may be copied, modified, propagated,
 * or distributed except according to the terms contained in the COPYING file.
 *
 * Copyright (c) 2000 Shlomi Fish
 */
// meta_alloc.h - the Freecell Solver compact allocator based on the
// meta-allocator concept that is used to collect the pages allocated by
// the standard allocator after it is destroyed and to recycle them.
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include "rinutils/likely.h"
#include "rinutils/typeof_wrap.h"
#include "state.h"

#ifndef FCS_IA_PACK_SIZE
#define FCS_IA_PACK_SIZE 64
#endif

typedef struct
{
    char *recycle_bin;
} meta_allocator;

typedef struct
{
    char *old_list;
    char *max_ptr;
    char *ptr;
    char *rollback_ptr;
    meta_allocator *meta;
} compact_allocator;

extern bool bh_solve_compact_allocator_extend(compact_allocator *);

/* To be called after the meta_alloc was set. */
static inline bool fc_solve_compact_allocator_init_helper(
    compact_allocator *const allocator)
{
    allocator->old_list = NULL;
    return bh_solve_compact_allocator_extend(allocator);
}

static inline void fc_solve_meta_compact_allocator_init(
    meta_allocator *const meta)
{
    meta->recycle_bin = NULL;
}

extern void bh_solve_meta_compact_allocator_finish(meta_allocator *);

static inline void *fcs_compact_alloc_ptr(
    compact_allocator *const allocator, const size_t how_much_proto)
{
    /* Round ptr to the next pointer boundary */
    const size_t how_much =
        how_much_proto +
        ((sizeof(char *) - ((how_much_proto) & (sizeof(char *) - 1))) &
            (sizeof(char *) - 1));

    if ((size_t)(allocator->max_ptr - allocator->ptr) < how_much)
    {
        if (unlikely(bh_solve_compact_allocator_extend(allocator)))
        {
            return NULL;
        }
    }
    else
    {
        allocator->rollback_ptr = allocator->ptr;
    }
    allocator->ptr += how_much;

    return allocator->rollback_ptr;
}

static inline void fcs_compact_alloc_release(compact_allocator *const allocator)
{
    allocator->ptr = allocator->rollback_ptr;
}

#define FCS__COMPACT_ALLOC__OLD_LIST_NEXT(ptr) (*((char **)(ptr)))
static inline void fc_solve_compact_allocator_finish(
    compact_allocator *const allocator)
{
    char *iter, *iter_next;
    meta_allocator *const meta = allocator->meta;
    var_AUTO(bin, meta->recycle_bin);
    // Enqueue all the allocated buffers in the meta allocator for re-use.
    for (iter = allocator->old_list,
        iter_next = FCS__COMPACT_ALLOC__OLD_LIST_NEXT(iter);
        iter_next;
        iter = iter_next, iter_next = FCS__COMPACT_ALLOC__OLD_LIST_NEXT(iter))
    {
        FCS__COMPACT_ALLOC__OLD_LIST_NEXT(iter) = bin;
        bin = iter;
    }

    FCS__COMPACT_ALLOC__OLD_LIST_NEXT(iter) = bin;
    meta->recycle_bin = iter;
}

static inline void fc_solve_compact_allocator_recycle(
    compact_allocator *const allocator)
{
    fc_solve_compact_allocator_finish(allocator);
    fc_solve_compact_allocator_init_helper(allocator);
}

static inline bool fc_solve_compact_allocator_init(
    compact_allocator *const allocator, meta_allocator *const meta_alloc)
{
    allocator->meta = meta_alloc;

    return fc_solve_compact_allocator_init_helper(allocator);
}

#ifdef __cplusplus
};
#endif
