/*
 * This file is part of Freecell Solver. It is subject to the license terms in
 * the COPYING.txt file found in the top-level directory of this distribution
 * and at http://fc-solve.shlomifish.org/docs/distro/COPYING.html . No part of
 * Freecell Solver, including this file, may be copied, modified, propagated,
 * or distributed except according to the terms contained in the COPYING file.
 *
 * Copyright (c) 2000 Shlomi Fish
 */
// fcs_hash.h - header file of Freecell Solver's internal hash implementation.
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "config.h"

#include "meta_alloc.h"
#include "state.h"

typedef unsigned bh_solve_hash_value_t;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
#ifdef USE_SYSTEM_XXHASH
#include "xxhash.h"
#if SIZEOF_VOID_P == 4
#define DO_XXH(b, l) XXH32((b), (l), 0)
#else
#define DO_XXH(b, l) XXH64((b), (l), 0)
#endif
#else
#include "wrap_xxhash.h"
#endif
#pragma clang diagnostic pop

static inline bh_solve_hash_value_t bh_solve__hash_function(
    const bhs_state_key_t key)
{
    return (bh_solve_hash_value_t)DO_XXH(&key, sizeof(key));
}
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
struct bh_solve_hash_symlink_item_struct
{
    // The next item in the list
    struct bh_solve_hash_symlink_item_struct *next;
    /* We also store the hash value corresponding to this key for faster
       comparisons */
    bh_solve_hash_value_t hash_value;
    // A pointer to the data structure that is to be collected
    bhs_state_key_t key;
#ifdef FCS_ENABLE_SECONDARY_HASH_VALUE
    // We also store a secondary hash value, which is not used for indexing,
    // but is used to speed up comparison.
    bh_solve_hash_value_t secondary_hash_value;
#endif
};
#pragma clang diagnostic pop

typedef struct bh_solve_hash_symlink_item_struct bh_solve_hash_symlink_item_t;

typedef struct
{
    bh_solve_hash_symlink_item_t *first_item;
} bh_solve_hash_symlink_t;

typedef struct
{
    /* The vector of the hash table itself */
    bh_solve_hash_symlink_t *entries;
#ifdef BHS_WITH_HASH_VACANT_ITEMS
    /* The list of vacant items as freed by the garbage collector. Use
     * if before allocating more. */
    bh_solve_hash_symlink_item_t *list_of_vacant_items;
#endif
    /* The size of the hash table */
    bh_solve_hash_value_t size;

    /* A bit mask that extract the lowest bits out of the hash value */
    bh_solve_hash_value_t size_bitmask;
    /* The number of elements stored inside the hash */
    bh_solve_hash_value_t num_elems;

    bh_solve_hash_value_t max_num_elems_before_resize;

    compact_allocator allocator;

} bh_solve_hash_t;

extern int bh_solve_hash_init(bh_solve_hash_t *hash, meta_allocator *);

// Returns false if the key is new and the key/val pair was inserted.
// Returns true if the key is not new and *existing_key / *existing_val
// was set to it.
extern int bh_solve_hash_insert(bh_solve_hash_t *hash, bhs_state_key_t *key);

static inline void bh_solve_hash_free(bh_solve_hash_t *hash)
{
    fc_solve_compact_allocator_finish(&(hash->allocator));

    free(hash->entries);
}

static inline void bh_solve_hash_recycle(bh_solve_hash_t *const hash)
{
    fc_solve_compact_allocator_recycle(&(hash->allocator));
    memset(hash->entries, '\0', sizeof(hash->entries[0]) * hash->size);
    hash->num_elems = 0;
}

#ifdef __cplusplus
}
#endif
