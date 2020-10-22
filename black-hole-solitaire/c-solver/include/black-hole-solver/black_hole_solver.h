// This file is part of Black Hole Solitaire Solver. It is subject to the
// license terms in the COPYING file found in the top-level directory of this
// distribution and at
// https://www.shlomifish.org/open-source/projects/black-hole-solitaire-solver/
// . No part of Black Hole Solitaire Solver, including this file, may be
// copied, modified, propagated, or distributed except according to the terms
// contained in the COPYING file.
//
// Copyright (c) 2010 Shlomi Fish

// black_hole_solver.h - a solver for Black Hole Solitaire - header of the API.
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <black-hole-solver/fcs_dllexport.h>

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
    black_hole_solver_instance_t *instance, const char *board_string,
    int *error_line_number, unsigned int num_columns,
    unsigned int max_num_cards_in_col, unsigned int bits_per_column);

DLLEXPORT extern int black_hole_solver_set_max_iters_limit(
    black_hole_solver_instance_t *instance, unsigned long limit);

DLLEXPORT extern int black_hole_solver_enable_place_queens_on_kings(
    black_hole_solver_instance_t *instance, bool enabled_status);

DLLEXPORT extern int black_hole_solver_enable_wrap_ranks(
    black_hole_solver_instance_t *instance, bool enabled_status);

DLLEXPORT extern int black_hole_solver_enable_rank_reachability_prune(
    black_hole_solver_instance_t *instance, bool enabled_status);

#define BLACK_HOLE_SOLVER__API__REQUIRES_SETUP_CALL 1
DLLEXPORT extern int black_hole_solver_config_setup(
    black_hole_solver_instance_t *instance);
DLLEXPORT extern int black_hole_solver_setup(
    black_hole_solver_instance_t *instance);

DLLEXPORT extern int black_hole_solver_run(
    black_hole_solver_instance_t *instance);

DLLEXPORT extern int black_hole_solver_recycle(
    black_hole_solver_instance_t *instance);

DLLEXPORT extern int black_hole_solver_free(
    black_hole_solver_instance_t *instance);

DLLEXPORT extern void black_hole_solver_init_solution_moves(
    black_hole_solver_instance_t *instance);
DLLEXPORT extern int black_hole_solver_get_next_move(
    black_hole_solver_instance_t *instance, int *col_idx_ptr,
    int *card_rank_ptr, int *card_suit_ptr /* Will return H=0, C=1, D=2, S=3 */
);

DLLEXPORT extern unsigned long black_hole_solver_get_num_states_in_collection(
    black_hole_solver_instance_t *instance);

DLLEXPORT extern unsigned long black_hole_solver_get_iterations_num(
    black_hole_solver_instance_t *instance);

// Added in version 1.10.0
DLLEXPORT extern unsigned long black_hole_solver_get_max_reached_depth(
    black_hole_solver_instance_t *instance);

DLLEXPORT extern int black_hole_solver_get_current_solution_board(
    black_hole_solver_instance_t *instance, char *output);

DLLEXPORT const char *black_hole_solver_get_lib_version(void);

#ifdef __cplusplus
}
#endif
