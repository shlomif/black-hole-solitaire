// This file is part of Black Hole Solitaire Solver. It is subject to the
// license terms in the COPYING file found in the top-level directory of this
// distribution and at
// https://www.shlomifish.org/open-source/projects/black-hole-solitaire-solver/
// . No part of Black Hole Solitaire Solver, including this file, may be
// copied, modified, propagated, or distributed except according to the terms
// contained in the COPYING file.
//
// Copyright (c) 2010 Shlomi Fish

// google_hash.h - header file for Google's dense_hash_map as adapted
// for Freecell Solver.
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#include "config.h"
#include "state.h"

#if (FCS_STATE_STORAGE == FCS_STATE_STORAGE_GOOGLE_DENSE_HASH)

typedef struct
{
    void *hash;
} bh_solve_hash_t;

extern void bh_solve_hash_init(bh_solve_hash_t *hash);

// Returns false if the key is new and the key/val pair was inserted.
// Returns true if the key is not new and *existing_key / *existing_val
// was set to it.
extern bool bh_solve_hash_insert(
    bh_solve_hash_t *hash, bhs_state_key_value_pair_t *key);

extern void bh_solve_hash_free(bh_solve_hash_t *hash);

extern void bh_solve_hash_get(bh_solve_hash_t *hash,
    bhs_state_key_value_pair_t *key_ptr, bhs_state_key_value_pair_t *result);

#endif

#ifdef __cplusplus
}
#endif
