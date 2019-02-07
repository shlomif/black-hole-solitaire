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
// black_hole_solver.h - a solver for Black Hole Solitaire - header of the API.
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <black-hole-solver/fcs_dllexport.h>
#include <black-hole-solver/bool.h>

enum
{
    BLACK_HOLE_SOLVER__SUCCESS = 0,
    BLACK_HOLE_SOLVER__OUT_OF_MEMORY,
    BLACK_HOLE_SOLVER__FOUNDATIONS_NOT_FOUND_AT_START,
    BLACK_HOLE_SOLVER__UNKNOWN_RANK,
    BLACK_HOLE_SOLVER__UNKNOWN_SUIT,
    BLACK_HOLE_SOLVER__TRAILING_CHARS,
    BLACK_HOLE_SOLVER__NOT_ENOUGH_COLUMNS,
    BLACK_HOLE_SOLVER__TOO_MANY_CARDS,
    BLACK_HOLE_SOLVER__NOT_SOLVABLE,
    BLACK_HOLE_SOLVER__END,
    BLACK_HOLE_SOLVER__OUT_OF_ITERS,
    BLACK_HOLE_SOLVER__INVALID_INPUT
};

typedef struct
{
    unsigned long nothing;
} black_hole_solver_instance_t;

DLLEXPORT extern int black_hole_solver_create(
    black_hole_solver_instance_t **ret_instance);

DLLEXPORT extern int black_hole_solver_read_board(
    black_hole_solver_instance_t *instance_proto, const char *board_string,
    int *error_line_number, unsigned int num_columns,
    unsigned int max_num_cards_in_col, unsigned int bits_per_column);

DLLEXPORT extern int black_hole_solver_set_max_iters_limit(
    black_hole_solver_instance_t *instance_proto, unsigned long limit);

DLLEXPORT extern int black_hole_solver_enable_place_queens_on_kings(
    black_hole_solver_instance_t *instance_proto, bool enabled_status);

DLLEXPORT extern int black_hole_solver_enable_wrap_ranks(
    black_hole_solver_instance_t *instance_proto, bool enabled_status);

DLLEXPORT extern int black_hole_solver_enable_rank_reachability_prune(
    black_hole_solver_instance_t *instance_proto, bool enabled_status);

DLLEXPORT extern int black_hole_solver_run(
    black_hole_solver_instance_t *ret_instance);

DLLEXPORT extern int black_hole_solver_free(
    black_hole_solver_instance_t *instance_proto);

DLLEXPORT extern int black_hole_solver_get_next_move(
    black_hole_solver_instance_t *instance_proto, int *col_idx_ptr,
    int *card_rank_ptr, int *card_suit_ptr /* Will return H=0, C=1, D=2, S=3 */
);

DLLEXPORT extern unsigned long black_hole_solver_get_num_states_in_collection(
    black_hole_solver_instance_t *instance_proto);

DLLEXPORT extern unsigned long black_hole_solver_get_iterations_num(
    black_hole_solver_instance_t *instance_proto);

DLLEXPORT extern int black_hole_solver_get_current_solution_board(
    black_hole_solver_instance_t *instance_proto, char *output);

DLLEXPORT const char *black_hole_solver_get_lib_version(void);

#ifdef __cplusplus
}
#endif
