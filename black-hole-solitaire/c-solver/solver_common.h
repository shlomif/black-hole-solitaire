// solver_common.h
// Copyright (C) 2018 Shlomi Fish <shlomif@cpan.org>
//
// Distributed under terms of the Expat license.

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
    GAME__ALL,
    GAME__GOLF
};

static inline void out_board(
    black_hole_solver_instance_t *const solver, const bool display_boards)
{
    if (!display_boards)
    {
        return;
    }

    char board[1000];
    if (black_hole_solver_get_current_solution_board(solver, board) ==
        BLACK_HOLE_SOLVER__SUCCESS)
    {
        printf("\n[START BOARD]\n%s[END BOARD]\n\n\n", board);
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

static inline int solver_run(black_hole_solver_instance_t *const solver,
    const unsigned long max_iters_limit,
    const unsigned long iters_display_step);

static inline int solve_filename(const char *const filename,
    enum GAME_TYPE game_type, unsigned long max_iters_limit,
    unsigned long iters_display_step, bool display_boards,
    bool is_rank_reachability_prune_enabled, bool place_queens_on_kings,
    bool quiet_output, bool wrap_ranks)
{
    int ret = 0;

    FILE *fh = stdin;
    if (filename)
    {
        fh = fopen(filename, "rt");
        if (!fh)
        {
            fprintf(stderr, "Cannot open '%s' for reading!\n", filename);
            return -1;
        }
    }
    char board[MAX_LEN_BOARD_STRING];
    fread(board, sizeof(board[0]), MAX_LEN_BOARD_STRING, fh);

    if (filename)
    {
        fclose(fh);
    }

    board[MAX_LEN_BOARD_STRING - 1] = '\0';
    black_hole_solver_instance_t *solver;
    if (black_hole_solver_create(&solver))
    {
        fputs("Could not initialise solver (out-of-memory)\n", stderr);
        exit(-1);
    }

    if (game_type == GAME__UNKNOWN)
    {
        fputs("Error! Must specify game type using --game.\n", stderr);
        exit(-1);
    }

    black_hole_solver_enable_rank_reachability_prune(
        solver, is_rank_reachability_prune_enabled);
    black_hole_solver_enable_wrap_ranks(solver, wrap_ranks);
    black_hole_solver_enable_place_queens_on_kings(
        solver, place_queens_on_kings);

    int error_line_num;
    const unsigned num_columns =
        ((game_type == GAME__BH)
                ? BHS__BLACK_HOLE__NUM_COLUMNS
                : (game_type == GAME__GOLF) ? BHS__GOLF__NUM_COLUMNS
                                            : BHS__ALL_IN_A_ROW__NUM_COLUMNS);
    if (black_hole_solver_read_board(solver, board, &error_line_num,
            num_columns,
            ((game_type == GAME__BH)
                    ? BHS__BLACK_HOLE__MAX_NUM_CARDS_IN_COL
                    : (game_type == GAME__GOLF)
                          ? BHS__GOLF__MAX_NUM_CARDS_IN_COL
                          : BHS__ALL_IN_A_ROW__MAX_NUM_CARDS_IN_COL),
            ((game_type == GAME__BH) ? BHS__BLACK_HOLE__BITS_PER_COL
                                     : BHS__GOLF__BITS_PER_COL)))
    {
        fprintf(stderr, "Error reading the board at line No. %d!\n",
            error_line_num);
        exit(-1);
    }

    const int solver_ret_code =
        solver_run(solver, max_iters_limit, iters_display_step);

    if (!solver_ret_code)
    {
        int col_idx, card_rank, card_suit;
        int next_move_ret_code;

        fputs("Solved!\n", stdout);

        if (!quiet_output)
        {
            out_board(solver, display_boards);

            while ((next_move_ret_code = black_hole_solver_get_next_move(
                        solver, &col_idx, &card_rank, &card_suit)) ==
                   BLACK_HOLE_SOLVER__SUCCESS)
            {
                if (col_idx == (int)num_columns)
                {
                    printf("%s", "Deal talon");
                }
                else
                {
                    printf("Move a card from stack %d to the foundations",
                        col_idx);
                }
                printf("\n\n"
                       "Info: Card moved is "
                       "%c%c\n\n\n====================\n\n",
                    (("0A23456789TJQK")[card_rank]), ("HCDS")[card_suit]);

                out_board(solver, display_boards);
            }

            if (next_move_ret_code != BLACK_HOLE_SOLVER__END)
            {
                fprintf(stderr, "%s - %d\n",
                    "Get next move routine returned the wrong error code.",
                    next_move_ret_code);
                ret = -1;
            }
        }
    }
    else if (solver_ret_code == BLACK_HOLE_SOLVER__OUT_OF_ITERS)
    {
        fputs("Intractable!\n", stdout);
        ret = -2;
    }
    else
    {
        fputs("Unsolved!\n", stdout);
        ret = -1;
    }

    printf("\n\n--------------------\n"
           "Total number of states checked is %lu.\n"
           "This scan generated %lu states.\n",
        black_hole_solver_get_iterations_num(solver),
        black_hole_solver_get_num_states_in_collection(solver));

    black_hole_solver_free(solver);
    return ret;
}
