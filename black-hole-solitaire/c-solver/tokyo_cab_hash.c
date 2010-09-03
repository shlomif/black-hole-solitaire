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
 * fcs_hash.c - an implementation of a simplistic (keys only) hash. This
 * hash uses chaining and re-hashing and was found to be very fast. Not all
 * of the functions of the hash ADT are implemented, but it is useful enough
 * for Freecell Solver.
 *
 */

#define BUILDING_DLL 1
#include "config.h"

#include <assert.h>
#include <stdlib.h>

#if (BHS_STATE_STORAGE == BHS_STATE_STORAGE_TOKYO_CAB_HASH)

#include "tokyo_cab_hash.h"
#include "state.h"

void bh_solve_hash_init(
    bh_solve_hash_t * hash
    )
{
    int ecode;

    hash->hash = tchdbnew();
    tchdbsetcache(hash->hash, 128*1024);
    if (!tchdbopen(hash->hash, "bh_solve.hdb", HDBOWRITER|HDBOTRUNC|HDBOCREAT))
    {
        ecode = tchdbecode(hash->hash);
        fprintf(stderr, "Tokyo Cabinet open error: %s\n", tchdberrmsg(ecode));
        exit(-1);
    }
    return;
}

fcs_bool_t bh_solve_hash_insert(
    bh_solve_hash_t * hash,
    bhs_state_key_value_pair_t * key
)
{
    int ecode;

    if (tchdbvsiz(hash->hash, &(key->key), sizeof(key->key)) < 0)
    {
        /* Record does not exist. */
        if (!tchdbputkeep(
            hash->hash,
            &(key->key),
            sizeof(key->key),
            &(key->value),
            sizeof(key->value)
            ))
        {
            ecode = tchdbecode(hash->hash);
            fprintf(stderr, "Tokyo Cabinet putkeep error: %s\n", tchdberrmsg(ecode));
            exit(-1);
    
        }
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

#else

/* ANSI C doesn't allow empty compilation */
static void bh_solve_hash_c_dummy();

#endif /* (FCS_STATE_STORAGE == FCS_STATE_STORAGE_INTERNAL_HASH) || defined(INDIRECT_STACK_STATES) */
