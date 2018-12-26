/*
 * main_common.h
 * Copyright (C) 2018 Shlomi Fish <shlomif@cpan.org>
 *
 * Distributed under terms of the Expat license.
 */
#pragma once

#include "min_and_max.h"
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <black-hole-solver/bool.h>
#include <black-hole-solver/black_hole_solver.h>
#include "state.h"

#include "config.h"

#define MAX_LEN_BOARD_STRING 4092

enum GAME_TYPE
{
    GAME__UNKNOWN = 0,
    GAME__BH,
    GAME__ALL
};

static void out_board(
    black_hole_solver_instance_t *const solver, const fcs_bool_t display_boards)
{
    if (!display_boards)
    {
        return;
    }

    char *board = NULL;
    black_hole_solver_get_current_solution_board(solver, &board);

    if (board)
    {
        printf("\n[START BOARD]\n%s[END BOARD]\n\n\n", board);
        free(board);
        board = NULL;
    }
}

static const char *const help_text =
    "black-hole-solve --game {all_in_a_row|black_hole} [more options] "
    "[/path/to/board_layout.txt]\n"
    "\n"
    "--help                        displays this help.\n"
    "--max-iters [iter_count]      limit the iterations.\n"
    "--game all_in_a_row           solve All in a Row games.\n"
    "--game black_hole             solve Black Hole games.\n"
    "--displays-boards             display the layout of the board at every "
    "step.\n"
    "--rank-reach-prune            enable the Rank Reachability Prune.\n"
    "--iters-display-step [step]   Display a trace every certain step.\n"
    "\n";
