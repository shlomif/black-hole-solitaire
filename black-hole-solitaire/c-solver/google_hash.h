/* Copyright (c) 2010 Shlomi Fish
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
 * google_hash.h - header file for Google's dense_hash_map as adapted
 * for Freecell Solver.
 */

#ifndef FC_SOLVE__GOOGLE_HASH_H
#define FC_SOLVE__GOOGLE_HASH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "config.h"

#include "bool.h"
#include "state.h"

#if (FCS_STATE_STORAGE == FCS_STATE_STORAGE_GOOGLE_DENSE_HASH)

typedef struct {
    void * hash;
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
    bhs_state_key_value_pair_t * key
    );

extern void bh_solve_hash_free(
    bh_solve_hash_t * hash
    );

extern void bh_solve_hash_get(
    bh_solve_hash_t * hash,
    bhs_state_key_value_pair_t * key_ptr,
    bhs_state_key_value_pair_t * result
    );

#endif


#ifdef __cplusplus
}
#endif

#endif /*  FC_SOLVE__GOOGLE_HASH_H */
