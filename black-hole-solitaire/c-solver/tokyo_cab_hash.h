// This file is part of Black Hole Solitaire Solver. It is subject to the
// license terms in the COPYING file found in the top-level directory of this
// distribution and at
// https://www.shlomifish.org/open-source/projects/black-hole-solitaire-solver/
// . No part of Black Hole Solitaire Solver, including this file, may be
// copied, modified, propagated, or distributed except according to the terms
// contained in the COPYING file.
//
// Copyright (c) 2010 Shlomi Fish

// tokyo_cab_hash.h - header file of the Tokyo Cabinet hash.
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <tcutil.h>
#include <tchdb.h>
#include <stdlib.h>
#include <stdbool.h>

#include "config.h"
#include "state.h"

typedef struct
{
    TCHDB *hash;
} bh_solve_hash_t;

static inline void bh_solve_hash_init(bh_solve_hash_t *const hash)
{
    hash->hash = tchdbnew();
    tchdbsetcache(hash->hash, 1024 * 1024);
    if (!tchdbopen(
            hash->hash, "bh_solve.hdb", HDBOWRITER | HDBOTRUNC | HDBOCREAT))
    {
        const int ecode = tchdbecode(hash->hash);
        fprintf(stderr, "Tokyo Cabinet open error: %s\n", tchdberrmsg(ecode));
        exit(-1);
    }
}

static inline bool bh_solve_hash_insert(
    bh_solve_hash_t *hash, bhs_state_key_value_pair_t *key)
{
    return (!tchdbputkeep(hash->hash, &(key->key), sizeof(key->key),
        &(key->value), sizeof(key->value)));
}

static inline void bh_solve_hash_free(bh_solve_hash_t *hash)
{
    tchdbclose(hash->hash);
    tchdbdel(hash->hash);
    hash->hash = NULL;
}

static inline void bh_solve_hash_get(bh_solve_hash_t *hash,
    bhs_state_key_value_pair_t *key_ptr, bhs_state_key_value_pair_t *result)
{
    result->key = key_ptr->key;
    tchdbget3(hash->hash, &(key_ptr->key), sizeof(key_ptr->key),
        &(result->value), sizeof(result->value));
}

#ifdef __cplusplus
}
#endif
