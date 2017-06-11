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
 * tokyo_cab_hash.h - header file of the Tokyo Cabinet hash.
 *
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <tcutil.h>
#include <tchdb.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "config.h"

#include <black-hole-solver/bool.h>
#include "state.h"

typedef struct
{
    TCHDB * hash;
} bh_solve_hash_t;

static inline void bh_solve_hash_init(
    bh_solve_hash_t * hash
    )
{
    int ecode;

    hash->hash = tchdbnew();
    tchdbsetcache(hash->hash, 1024*1024);
    if (!tchdbopen(hash->hash, "bh_solve.hdb", HDBOWRITER|HDBOTRUNC|HDBOCREAT))
    {
        ecode = tchdbecode(hash->hash);
        fprintf(stderr, "Tokyo Cabinet open error: %s\n", tchdberrmsg(ecode));
        exit(-1);
    }
}


static inline fcs_bool_t bh_solve_hash_insert(
    bh_solve_hash_t * hash,
    bhs_state_key_value_pair_t * key
)
{
    return (!tchdbputkeep(
        hash->hash,
        &(key->key),
        sizeof(key->key),
        &(key->value),
        sizeof(key->value)
    ));
}

/*
 * Returns FALSE if the key is new and the key/val pair was inserted.
 * Returns TRUE if the key is not new.
 */
extern fcs_bool_t bh_solve_hash_insert(
    bh_solve_hash_t * hash,
    bhs_state_key_value_pair_t * key
    );

static inline void bh_solve_hash_free(
    bh_solve_hash_t * hash
    )
{
    tchdbclose(hash->hash);
    tchdbdel(hash->hash);
    hash->hash = NULL;
}

static inline void bh_solve_hash_get(
    bh_solve_hash_t * hash,
    bhs_state_key_value_pair_t * key_ptr,
    bhs_state_key_value_pair_t * result
    )
{
    result->key = key_ptr->key;
    tchdbget3(hash->hash, &(key_ptr->key), sizeof(key_ptr->key), &(result->value), sizeof(result->value));
}

#ifdef __cplusplus
}
#endif
