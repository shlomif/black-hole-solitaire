// This file is part of Black Hole Solitaire Solver. It is subject to the
// license terms in the COPYING file found in the top-level directory of this
// distribution and at
// https://www.shlomifish.org/open-source/projects/black-hole-solitaire-solver/
// . No part of Black Hole Solitaire Solver, including this file, may be
// copied, modified, propagated, or distributed except according to the terms
// contained in the COPYING file.
//
// Copyright (c) 2010 Shlomi Fish

// fcs_hash.c - an implementation of a simplistic (keys only) hash. This
// hash uses chaining and re-hashing and was found to be very fast. Not all
// of the functions of the hash ADT are implemented, but it is useful enough
// for Freecell Solver.

#include "config.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "rinutils/typeof_wrap.h"
#include "fcs_hash.h"
#include "state.h"

// This function "rehashes" a hash. I.e: it increases the size of its
// hash table, allowing for smaller chains, and faster lookup.
static inline bool bh_solve_hash_rehash(bh_solve_hash_t *hash)
{
    const_AUTO(old_size, hash->size);

    const_AUTO(new_size, old_size << 1);
    const_AUTO(new_size_bitmask, new_size - 1);

    bh_solve_hash_symlink_t *new_entries;
    if (unlikely(
            !(new_entries = calloc((size_t)new_size, sizeof(new_entries[0])))))
    {
        return true;
    }

    /* Copy the items to the new hash while not allocating them again */
    for (bh_solve_hash_value_t i = 0; i < old_size; ++i)
    {
        var_AUTO(item, hash->entries[i].first_item);
        /* traverse the chain item by item */
        while (item != NULL)
        {
            /* The place in the new hash table */
            const_AUTO(place, item->hash_value & new_size_bitmask);

            /* Store the next item in the linked list in a safe place,
               so we can retrieve it after the assignment */
            const_AUTO(next_item, item->next);
            /* It is placed in front of the first element in the chain,
               so it should link to it */
            item->next = new_entries[place].first_item;

            /* Make it the first item in its chain */
            new_entries[place].first_item = item;

            /* Move to the next item this one. */
            item = next_item;
        }
    };

    /* Free the entries of the old hash */
    free(hash->entries);

    /* Copy the new hash to the old one */
    hash->entries = new_entries;
    hash->size = new_size;
    hash->size_bitmask = new_size_bitmask;
    hash->max_num_elems_before_resize = (new_size << 1);
    return false;
}

bool bh_solve_hash_init(bh_solve_hash_t *hash, meta_allocator *const meta_alloc)
{
    const bh_solve_hash_value_t size = 256;

    hash->size = size;
    hash->size_bitmask = size - 1;
    hash->max_num_elems_before_resize = (size << 1);

    hash->num_elems = 0;

    /* Allocate a table of size entries */
    if (!(hash->entries =
                calloc((size_t)size, sizeof(bh_solve_hash_symlink_t))))
    {
        return true;
    }
#ifdef BHS_WITH_HASH_VACANT_ITEMS
    hash->list_of_vacant_items = NULL;
#endif

    if (unlikely(
            fc_solve_compact_allocator_init(&(hash->allocator), meta_alloc)))
    {
        free(hash->entries);
        hash->entries = NULL;
        return true;
    }

    return false;
}

int bh_solve_hash_insert(
    bh_solve_hash_t *const hash, bhs_state_key_value_pair_t *const key)
{
    bh_solve_hash_symlink_item_t *item, *last_item;
    bh_solve_hash_symlink_item_t **item_placeholder;
#ifdef FCS_INLINED_HASH_COMPARISON
    enum FCS_INLINED_HASH_DATA_TYPE hash_type;
#endif

    const_AUTO(hash_value, bh_solve__hash_function(key->key));

#ifdef FCS_INLINED_HASH_COMPARISON
    hash_type = hash->hash_type;
#endif
    /* Get the index of the appropriate chain in the hash table */
#define PLACE() (hash_value & (hash->size_bitmask))

    bh_solve_hash_symlink_t *list = (hash->entries + PLACE());

#undef PLACE

    /* If first_item is non-existent */
    if (list->first_item == NULL)
    {
        /* Allocate a first item with that key/val pair */
        item_placeholder = &(list->first_item);
    }
    else
    {
        /* Initialize item to the chain's first_item */
        item = list->first_item;
        last_item = NULL;

        while (item != NULL)
        {
            if ((!memcmp(
                    &(item->key.key), &(key->key), sizeof(bhs_state_key_t))))
            {
                return 1;
            }
            /* Cache the item before the current in last_item */
            last_item = item;
            /* Move to the next item */
            item = item->next;
        }

        item_placeholder = &(last_item->next);
    }

#ifdef BHS_WITH_HASH_VACANT_ITEMS
    if (hash->list_of_vacant_items)
    {
        hash->list_of_vacant_items = (item = hash->list_of_vacant_items)->next;
    }
    else
    {
#endif
        if (unlikely(!(item = fcs_compact_alloc_ptr(
                           &(hash->allocator), sizeof(*item)))))
        {
            return -1;
        }
#ifdef BHS_WITH_HASH_VACANT_ITEMS
    }
#endif

    *(item_placeholder) = item;

    /* Put the new element at the end of the list */
    /* Do an in-order insertion. */
    *item = (bh_solve_hash_symlink_item_t){
        .key = (*key), .hash_value = hash_value, .next = NULL};
#ifdef FCS_ENABLE_SECONDARY_HASH_VALUE
    item->secondary_hash_value = secondary_hash_value;
#endif

    if ((++hash->num_elems) > hash->max_num_elems_before_resize)
    {
        if (unlikely(bh_solve_hash_rehash(hash)))
        {
            return -1;
        }
    }

    return 0;
}
