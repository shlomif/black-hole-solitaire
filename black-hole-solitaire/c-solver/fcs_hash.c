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

#include "fcs_hash.h"
#include "state.h"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
#include "wrap_xxhash.h"
#pragma clang diagnostic pop

static inline unsigned long hash_function(const bhs_state_key_t key)
{
    return DO_XXH(&key, sizeof(key));
}

static inline void bh_solve_hash_rehash(bh_solve_hash_t *hash);

void bh_solve_hash_init(bh_solve_hash_t *hash, meta_allocator *const meta_alloc)
{
    const int size = 256;

    hash->size = size;
    hash->size_bitmask = size - 1;
    hash->max_num_elems_before_resize = (size << 1);

    hash->num_elems = 0;

    /* Allocate a table of size entries */
    hash->entries = (bh_solve_hash_symlink_t *)malloc(
        sizeof(bh_solve_hash_symlink_t) * (size_t)size);

    hash->list_of_vacant_items = NULL;

    /* Initialize all the cells of the hash table to NULL, which indicate
       that the cork of the linked list is right at the start */
    memset(hash->entries, 0, sizeof(bh_solve_hash_symlink_t) * ((size_t)size));

    fc_solve_compact_allocator_init(&(hash->allocator), meta_alloc);
}

void bh_solve_hash_get(
    bh_solve_hash_t *hash, bhs_state_key_t *key_ptr, bhs_state_value_t *result)
{
    bh_solve_hash_symlink_t *list;
    bh_solve_hash_symlink_item_t *item;

    bh_solve_hash_value_t hash_value =
        ((typeof(hash_value))hash_function(*key_ptr));

#define PLACE() (hash_value & (hash->size_bitmask))
    list = (hash->entries + PLACE());

    item = list->first_item;

    assert(item != NULL);

    while (item != NULL)
    {
        if (!memcmp(&(item->key.key), key_ptr, sizeof(bhs_state_key_t)))
        {
            *result = item->key.value;
            return;
        }

        item = item->next;
    }

    assert(false);
}

bool bh_solve_hash_insert(
    bh_solve_hash_t *const hash, bhs_state_key_value_pair_t *const key)
{
    bh_solve_hash_symlink_t *list;
    bh_solve_hash_symlink_item_t *item, *last_item;
    bh_solve_hash_symlink_item_t **item_placeholder;
#ifdef FCS_INLINED_HASH_COMPARISON
    enum FCS_INLINED_HASH_DATA_TYPE hash_type;
#endif

    bh_solve_hash_value_t hash_value =
        ((typeof(hash_value))hash_function(key->key));

#ifdef FCS_INLINED_HASH_COMPARISON
    hash_type = hash->hash_type;
#endif
    /* Get the index of the appropriate chain in the hash table */
#define PLACE() (hash_value & (hash->size_bitmask))

    list = (hash->entries + PLACE());

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
                return true;
            }
            /* Cache the item before the current in last_item */
            last_item = item;
            /* Move to the next item */
            item = item->next;
        }

        item_placeholder = &(last_item->next);
    }

    if (hash->list_of_vacant_items)
    {
        hash->list_of_vacant_items = (item = hash->list_of_vacant_items)->next;
    }
    else
    {
        item = fcs_compact_alloc_ptr(&(hash->allocator), sizeof(*item));
    }

    *(item_placeholder) = item;

    /* Put the new element at the end of the list */
    /* Do an in-order insertion. */
    item->key = (*key);
    item->hash_value = hash_value;
#ifdef FCS_ENABLE_SECONDARY_HASH_VALUE
    item->secondary_hash_value = secondary_hash_value;
#endif
    item->next = NULL;

    if ((++hash->num_elems) > hash->max_num_elems_before_resize)
    {
        bh_solve_hash_rehash(hash);
    }

    return false;
}

/*
    This function "rehashes" a hash. I.e: it increases the size of its
    hash table, allowing for smaller chains, and faster lookup.

  */
static inline void bh_solve_hash_rehash(bh_solve_hash_t *hash)
{
    int old_size, new_size_bitmask;
    bh_solve_hash_symlink_item_t *item, *next_item;
    int place;
    bh_solve_hash_symlink_t *new_entries;

    old_size = hash->size;

    const int new_size = old_size << 1;
    new_size_bitmask = new_size - 1;

    new_entries = calloc((size_t)new_size, sizeof(bh_solve_hash_symlink_t));

    /* Copy the items to the new hash while not allocating them again */
    for (int i = 0; i < old_size; i++)
    {
        item = hash->entries[i].first_item;
        /* traverse the chain item by item */
        while (item != NULL)
        {
            /* The place in the new hash table */
            place = item->hash_value & new_size_bitmask;

            /* Store the next item in the linked list in a safe place,
               so we can retrieve it after the assignment */
            next_item = item->next;
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
}
