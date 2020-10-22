// solver_common.h
// Copyright (C) 2018 Shlomi Fish <shlomif@cpan.org>
//
// Distributed under terms of the Expat license.

#pragma once

#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <black-hole-solver/black_hole_solver.h>
#include "state.h"
#include "rinutils/likely.h"

#include "config.h"

#define MAX_LEN_BOARD_STRING 4092

enum GAME_TYPE
{
    GAME__UNKNOWN = 0,
    GAME__BH,
    GAME__ALL,
    GAME__GOLF
};

static inline void out_board(FILE *out_fh,
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
        fprintf(out_fh, "\n[START BOARD]\n%s[END BOARD]\n\n\n", board);
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

#include "solver_run.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
typedef struct
{
    black_hole_solver_instance_t *the_solver;
    FILE *out_fh;
    unsigned long iters_display_step;
    unsigned long max_iters_limit;
    enum GAME_TYPE game_type;
    bool display_boards;
    bool is_rank_reachability_prune_enabled;
    bool place_queens_on_kings;
    bool quiet_output;
    bool wrap_ranks;
    bool show_max_reached_depth;
} bhs_settings;
#pragma clang diagnostic pop

static inline bhs_settings parse_cmd_line(
    int argc, char *argv[], int *out_arg_idx)
{
    bhs_settings settings;
    settings.out_fh = stdout;
    settings.iters_display_step = 0;
    settings.game_type = GAME__UNKNOWN;
    settings.display_boards = false;
    settings.is_rank_reachability_prune_enabled = false;
    settings.place_queens_on_kings = false;
    settings.quiet_output = false;
    settings.wrap_ranks = true;
    settings.max_iters_limit = ULONG_MAX;
    settings.show_max_reached_depth = false;

    int arg_idx = 1;
    while (argc > arg_idx)
    {
        if (!strcmp(argv[arg_idx], "--version"))
        {
            printf("black-hole-solver version %s\nLibrary version %s\n",
                VERSION, VERSION);
            exit(0);
        }
        else if (!strcmp(argv[arg_idx], "--help"))
        {
            printf("%s", help_text);
            exit(0);
        }
        else if (!strcmp(argv[arg_idx], "--output"))
        {
            if (argc == ++arg_idx)
            {
                fputs("Error! --output requires an argument.\n", stderr);
                exit(-1);
            }
            settings.out_fh = fopen(argv[arg_idx++], "wt");
        }
        else if (!strcmp(argv[arg_idx], "--max-iters"))
        {
            if (argc == ++arg_idx)
            {
                fputs("Error! --max-iters requires an argument.\n", stderr);
                exit(-1);
            }
            settings.max_iters_limit = (unsigned long)atol(argv[arg_idx++]);
        }
        else if (!strcmp(argv[arg_idx], "--game"))
        {
            if (argc == ++arg_idx)
            {
                fputs("Error! --game requires an argument.\n", stderr);
                exit(-1);
            }
            char *g = argv[arg_idx++];

            if (!strcmp(g, "black_hole"))
            {
                settings.game_type = GAME__BH;
            }
            else if (!strcmp(g, "golf"))
            {
                settings.game_type = GAME__GOLF;
                settings.wrap_ranks = false;
            }
            else if (!strcmp(g, "all_in_a_row"))
            {
                settings.game_type = GAME__ALL;
            }
            else
            {
                fprintf(stderr, "%s\n",
                    "Error! --game should be either \"black_hole\" or "
                    "\"all_in_a_row\".");
                exit(-1);
            }
        }
        else if (!strcmp(argv[arg_idx], "--no-queens-on-kings"))
        {
            ++arg_idx;
            settings.place_queens_on_kings = false;
        }
        else if (!strcmp(argv[arg_idx], "--queens-on-kings"))
        {
            ++arg_idx;
            settings.place_queens_on_kings = true;
        }
        else if (!strcmp(argv[arg_idx], "--show-max-reached-depth"))
        {
            ++arg_idx;
            settings.show_max_reached_depth = true;
        }
        else if (!strcmp(argv[arg_idx], "--no-wrap-ranks"))
        {
            ++arg_idx;
            settings.wrap_ranks = false;
        }
        else if (!strcmp(argv[arg_idx], "--quiet"))
        {
            ++arg_idx;
            settings.quiet_output = true;
        }
        else if (!strcmp(argv[arg_idx], "--wrap-ranks"))
        {
            ++arg_idx;
            settings.wrap_ranks = true;
        }
        else if (!strcmp(argv[arg_idx], "--display-boards"))
        {
            ++arg_idx;
            settings.display_boards = true;
        }
        else if (!strcmp(argv[arg_idx], "--rank-reach-prune"))
        {
            ++arg_idx;
            settings.is_rank_reachability_prune_enabled = true;
        }
        else if (!strcmp(argv[arg_idx], "--iters-display-step"))
        {
            if (argc == ++arg_idx)
            {
                fprintf(stderr,
                    "Error! --iters-display-step requires an arguments.\n");
                exit(-1);
            }
            settings.iters_display_step = (unsigned long)atol(argv[arg_idx++]);
        }
        else
        {
            break;
        }
    }
    if (settings.game_type == GAME__UNKNOWN)
    {
        fputs("Error! Must specify game type using --game.\n", stderr);
        exit(-1);
    }
    *out_arg_idx = arg_idx;
#define solver (settings_ptr->the_solver)
    bhs_settings *settings_ptr = &settings;
    if (black_hole_solver_create(&solver))
    {
        fputs("Could not initialise solver (out-of-memory)\n", stderr);
        exit(-1);
    }

    black_hole_solver_enable_rank_reachability_prune(
        solver, settings.is_rank_reachability_prune_enabled);
    black_hole_solver_enable_wrap_ranks(solver, settings.wrap_ranks);
    black_hole_solver_enable_place_queens_on_kings(
        solver, settings.place_queens_on_kings);
    black_hole_solver_config_setup(solver);

    return settings;
}

static inline int solve_filename(
    const char *const filename, bhs_settings *const settings_ptr)
{
#define settings (*settings_ptr)
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

    int error_line_num;
    const enum GAME_TYPE game_type = settings.game_type;
    const unsigned num_columns =
        ((game_type == GAME__BH)        ? BHS__BLACK_HOLE__NUM_COLUMNS
            : (game_type == GAME__GOLF) ? BHS__GOLF__NUM_COLUMNS
                                        : BHS__ALL_IN_A_ROW__NUM_COLUMNS);
    if (black_hole_solver_read_board(solver, board, &error_line_num,
            num_columns,
            ((game_type == GAME__BH) ? BHS__BLACK_HOLE__MAX_NUM_CARDS_IN_COL
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
    if (unlikely(black_hole_solver_setup(solver)))
    {
        fputs("Could not initialise solver (out-of-memory)\n", stderr);
        exit(-1);
    }

    const int solver_ret_code = solver_run(
        solver, settings.max_iters_limit, settings.iters_display_step);
    FILE *const out_fh = settings_ptr->out_fh;

    if (!solver_ret_code)
    {
        int col_idx, card_rank, card_suit;
        int next_move_ret_code;

        fputs("Solved!\n", out_fh);

        if (!settings.quiet_output)
        {
            black_hole_solver_init_solution_moves(solver);
            out_board(out_fh, solver, settings.display_boards);

            while ((next_move_ret_code = black_hole_solver_get_next_move(
                        solver, &col_idx, &card_rank, &card_suit)) ==
                   BLACK_HOLE_SOLVER__SUCCESS)
            {
                if (col_idx == (int)num_columns)
                {
                    fprintf(out_fh, "%s", "Deal talon");
                }
                else
                {
                    fprintf(out_fh,
                        "Move a card from stack %d to the foundations",
                        col_idx);
                }
                fprintf(out_fh,
                    "\n\n"
                    "Info: Card moved is "
                    "%c%c\n\n\n====================\n\n",
                    (("0A23456789TJQK")[card_rank]), ("HCDS")[card_suit]);

                out_board(out_fh, solver, settings.display_boards);
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
    else if (solver_ret_code == BLACK_HOLE_SOLVER__OUT_OF_MEMORY)
    {
        fputs("Out of memory!\n", stderr);
        exit(-1);
    }
    else if (solver_ret_code == BLACK_HOLE_SOLVER__OUT_OF_ITERS)
    {
        fputs("Intractable!\n", out_fh);
        ret = -2;
    }
    else
    {
        fputs("Unsolved!\n", out_fh);
        ret = -1;
    }

    fprintf(out_fh,
        "\n\n--------------------\n"
        "Total number of states checked is %lu.\n"
        "This scan generated %lu states.\n",
        black_hole_solver_get_iterations_num(solver),
        black_hole_solver_get_num_states_in_collection(solver));
    if (settings.show_max_reached_depth)
    {
        fprintf(out_fh, "Reached a maximal depth of %lu.\n",
            black_hole_solver_get_max_reached_depth(solver));
    }

    black_hole_solver_recycle(solver);
    return ret;
}

static inline void solve_free(bhs_settings *const settings_ptr)
{
    if (settings_ptr->out_fh != stdout)
    {
        fclose(settings_ptr->out_fh);
    }
    black_hole_solver_free(solver);
}
#undef solver
#undef settings
