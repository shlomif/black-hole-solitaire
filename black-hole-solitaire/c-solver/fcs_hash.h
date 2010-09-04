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
 * fcs_hash.h - header file of Freecell Solver's internal hash implementation.
 *
 */

#ifndef FC_SOLVE__FCS_HASH_H
#define FC_SOLVE__FCS_HASH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "config.h"

#include "alloc.h"

#include "inline.h"
#include "bool.h"
#include "state.h"

#ifdef FCS_INLINED_HASH_COMPARISON
enum FCS_INLINED_HASH_DATA_TYPE
{
    FCS_INLINED_HASH__COLUMNS,
    FCS_INLINED_HASH__STATES,
};
#endif

typedef int bh_solve_hash_value_t;

struct bh_solve_hash_symlink_item_struct
{
    /* A pointer to the data structure that is to be collected */
    bhs_state_key_value_pair_t key;
    /* We also store the hash value corresponding to this key for faster
       comparisons */
    bh_solve_hash_value_t hash_value;
#ifdef FCS_ENABLE_SECONDARY_HASH_VALUE
    /*
     * We also store a secondary hash value, which is not used for indexing,
     * but is used to speed up comparison.
     * */
    bh_solve_hash_value_t secondary_hash_value;
#endif
    /* The next item in the list */
    struct bh_solve_hash_symlink_item_struct * next;
};

typedef struct bh_solve_hash_symlink_item_struct bh_solve_hash_symlink_item_t;

typedef struct 
{
    bh_solve_hash_symlink_item_t * first_item;
} bh_solve_hash_symlink_t;

typedef struct
{
    /* The vector of the hash table itself */
    bh_solve_hash_symlink_t * entries;
    /* The list of vacant items as freed by the garbage collector. Use
     * if before allocating more. */
    bh_solve_hash_symlink_item_t * list_of_vacant_items;
    /* A comparison function that can be used for comparing two keys
       in the collection */
    /* The size of the hash table */
    int size;

    /* A bit mask that extract the lowest bits out of the hash value */
    int size_bitmask;
    /* The number of elements stored inside the hash */
    int num_elems;

    int max_num_elems_before_resize;

    bhs_compact_allocator_t allocator;

} bh_solve_hash_t;


extern void
bh_solve_hash_init(
    bh_solve_hash_t * hash
    );

/*
 * Returns FALSE if the key is new and the key/val pair was inserted.
 * Returns TRUE if the key is not new and *existing_key / *existing_val
 * was set to it.
 */
extern fcs_bool_t bh_solve_hash_insert(
    bh_solve_hash_t * hash,
    bhs_state_key_value_pair_t * key,
    bh_solve_hash_value_t hash_value
#ifdef FCS_ENABLE_SECONDARY_HASH_VALUE
    , bh_solve_hash_value_t secondary_hash_value
#endif
    );



static GCC_INLINE void bh_solve_hash_free(
    bh_solve_hash_t * hash
    )
{
    bh_solve_compact_allocator_finish(&(hash->allocator));

    free(hash->entries);
}

extern void bh_solve_hash_get(
    bh_solve_hash_t * hash,
    bhs_state_key_value_pair_t * key_ptr,
    bhs_state_key_value_pair_t * result,
    bh_solve_hash_value_t hash_value
    );


#ifdef __cplusplus
}
#endif

#endif /* FC_SOLVE__FCS_HASH_H */




