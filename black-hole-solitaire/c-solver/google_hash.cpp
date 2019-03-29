// This file is part of Black Hole Solitaire Solver. It is subject to the
// license terms in the COPYING file found in the top-level directory of this
// distribution and at
// https://www.shlomifish.org/open-source/projects/black-hole-solitaire-solver/
// . No part of Black Hole Solitaire Solver, including this file, may be
// copied, modified, propagated, or distributed except according to the terms
// contained in the COPYING file.
//
// Copyright (c) 2010 Shlomi Fish

// google_hash.cpp - module file for Google's sparse_hash_map as adapted
// for Freecell Solver.

#include "google_hash.h"

#include <google/sparse_hash_map>

using google::sparse_hash_map; // namespace where class lives by default

typedef unsigned long int ub4; /* unsigned 4-byte quantities */
typedef unsigned char ub1;

static inline ub4 perl_hash_function(register ub1 *s_ptr, /* the key */
    register ub4 length /* the length of the key */
)
{
    register ub4 hash_value_int = 0;
    register ub1 *s_end = s_ptr + length;

    while (s_ptr < s_end)
    {
        hash_value_int += (hash_value_int << 5) + *(s_ptr++);
    }
    hash_value_int += (hash_value_int >> 5);

    return hash_value_int;
}

struct state_equality
{
    bool operator()(bhs_state_key_t k1, bhs_state_key_t k2) const
    {
        return (!memcmp(&k1, &k2, sizeof(k1)));
    }
};

struct state_hash
{
    int operator()(bhs_state_key_t k1) const
    {
        return perl_hash_function((ub1 *)(&k1), sizeof(k1));
    }
};

typedef sparse_hash_map<bhs_state_key_t, bhs_state_value_t, state_hash,
    state_equality>
    StatesGoogleHash;

extern "C" void bh_solve_hash_init(bh_solve_hash_t *hash)
{
    StatesGoogleHash *ret = new StatesGoogleHash;

    hash->hash = (void *)ret;

    return;
}

/*
 * Returns 0 if the key is new and the key/val pair was inserted.
 *      - in that case *existing_key / *existing_val will be set to key
 *      and val respectively.
 * Returns 1 if the key is not new and *existing_key / *existing_val
 * was set to it.
 */
extern "C" bool bh_solve_hash_insert(
    bh_solve_hash_t *void_hash, bhs_state_key_value_pair_t *key)
{
    StatesGoogleHash *hash = (StatesGoogleHash *)(void_hash->hash);
    std::pair<StatesGoogleHash::iterator, bool> result =
        hash->insert(std::make_pair(key->key, key->value));

    /* If an insertion took place. */
    return (!(result.second));
}

extern "C" void bh_solve_hash_free(bh_solve_hash_t *void_hash)
{
    StatesGoogleHash *hash = (StatesGoogleHash *)void_hash->hash;

    delete hash;

    void_hash->hash = NULL;

    return;
}

extern "C" void bh_solve_hash_get(bh_solve_hash_t *void_hash,
    bhs_state_key_value_pair_t *key_ptr, bhs_state_key_value_pair_t *result)
{
    StatesGoogleHash *hash = (StatesGoogleHash *)void_hash->hash;
    StatesGoogleHash::iterator it = hash->find(key_ptr->key);

    result->key = key_ptr->key;
    result->value = ((*(it)).second);
}
