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
 * black_hole_solver_main.c - a solver for Black Hole Solitaire - header
 * of the command line program.
 */

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
    black_hole_solver_instance_t * solver,
    fcs_bool_t display_boards
)
{
    char * board = NULL;

    if (! display_boards)
    {
        return;
    }

    black_hole_solver_get_current_solution_board(
        solver,
        &board
    );

    if (board)
    {
        printf("\n[START BOARD]\n%s[END BOARD]\n\n\n", board);
        free(board);
        board = NULL;
    }

    return;
}

static const char const * help_text =
"black-hole-solve --game {all_in_a_row|black_hole} [more options] [/path/to/board_layout.txt]\n"
"\n"
"--help                        displays this help.\n"
"--max-iters [iter_count]      limit the iterations.\n"
"--game all_in_a_row           solve All in a Row games.\n"
"--game black_hole             solve Black Hole games.\n"
"--displays-boards             display the layout of the board at every step.\n"
"--rank-reach-prune            enable the Rank Reachability Prune.\n"
"--iters-display-step [step]   Display a trace every certain step.\n"
"\n"
    ;

int main(int argc, char * argv[])
{
    black_hole_solver_instance_t * solver;
    char board[MAX_LEN_BOARD_STRING];
    int error_line_num;
    int ret, solver_ret_code;
    char * filename = NULL;
    FILE * fh;
    int arg_idx;
    long max_iters_limit = -1;
    long iters_display_step = 0;
    enum GAME_TYPE game_type = GAME__UNKNOWN;
    fcs_bool_t display_boards = FALSE;
    fcs_bool_t is_rank_reachability_prune_enabled = FALSE;

    arg_idx = 1;
    while (argc > arg_idx)
    {
        if (!strcmp(argv[arg_idx], "--version"))
        {
            printf("black-hole-solver version %s\nLibrary version %s\n",
                VERSION,
                VERSION
            );
            exit(0);
        }
        else if (!strcmp(argv[arg_idx], "--help"))
        {
            printf ("%s", help_text);
            exit(0);
        }
        else if (!strcmp(argv[arg_idx], "--max-iters"))
        {
            arg_idx++;
            if (argc == arg_idx)
            {
                fprintf(stderr, "Error! --max-iters requires an arguments.\n");
                exit(-1);
            }
            max_iters_limit = atol(argv[arg_idx++]);
        }
        else if (!strcmp(argv[arg_idx], "--game"))
        {
            arg_idx++;
            if (argc == arg_idx)
            {
                fprintf(stderr, "Error! --game requires an arguments.\n");
                exit(-1);
            }
            char * g = argv[arg_idx++];

            if (!strcmp(g, "black_hole"))
            {
                game_type = GAME__BH;
            }
            else if (!strcmp(g, "all_in_a_row"))
            {
                game_type = GAME__ALL;
            }
            else
            {
                fprintf(stderr, "%s\n", "Error! --game should be either \"black_hole\" or \"all_in_a_row\".");
                exit(-1);
            }
        }
        else if (!strcmp(argv[arg_idx], "--display-boards"))
        {
            arg_idx++;
            display_boards = TRUE;
        }
        else if (!strcmp(argv[arg_idx], "--rank-reach-prune"))
        {
            arg_idx++;
            is_rank_reachability_prune_enabled = TRUE;
        }
        else if (!strcmp(argv[arg_idx], "--iters-display-step"))
        {
            arg_idx++;
            if (argc == arg_idx)
            {
                fprintf(stderr, "Error! --iters-display-step requires an arguments.\n");
                exit(-1);
            }
            iters_display_step = atol(argv[arg_idx++]);

            if (iters_display_step < 0)
            {
                fprintf(stderr, "Error! --iters-display-step should be positive or zero.\n");
                exit(-1);
            }
        }
        else
        {
            break;
        }
    }

    if (black_hole_solver_create(&solver))
    {
        fprintf(stderr, "%s\n", "Could not initialise solver (out-of-memory)");
        exit(-1);
    }

    if (game_type == GAME__UNKNOWN)
    {
        fprintf(stderr, "%s\n", "Error! Must specify game type using --game.");
        exit(-1);
    }

    max_iters_limit = iters_display_step;
    black_hole_solver_set_max_iters_limit(solver, max_iters_limit);
    black_hole_solver_enable_rank_reachability_prune(
        solver,
        is_rank_reachability_prune_enabled
    );

    if (argc > arg_idx)
    {
        if (strcmp(argv[arg_idx], "-"))
        {
            filename = argv[arg_idx];
        }
        arg_idx++;
    }

    if (filename)
    {
        fh = fopen(filename, "rt");
    }
    else
    {
        fh = stdin;
    }

    fread(board, sizeof(board[0]), MAX_LEN_BOARD_STRING, fh);

    if (filename)
    {
        fclose(fh);
    }

    board[MAX_LEN_BOARD_STRING-1] = '\0';

    if (black_hole_solver_read_board(
        solver,
        board,
        &error_line_num,
        (
            (game_type == GAME__BH)
            ? BHS__BLACK_HOLE__NUM_COLUMNS
            : BHS__ALL_IN_A_ROW__NUM_COLUMNS
        ),
        (
            (game_type == GAME__BH)
            ? BHS__BLACK_HOLE__MAX_NUM_CARDS_IN_COL
            : BHS__ALL_IN_A_ROW__MAX_NUM_CARDS_IN_COL
        ),
        (
            (game_type == GAME__BH)
            ? BHS__BLACK_HOLE__BITS_PER_COL
            : BHS__ALL_IN_A_ROW__BITS_PER_COL
        )
        )
    )
    {
        fprintf(stderr, "Error reading the board at line No. %d!\n", error_line_num);
        exit(-1);
    }

    ret = 0;

    do
    {
        solver_ret_code = black_hole_solver_run(solver);
        if (max_iters_limit == black_hole_solver_get_iterations_num(solver))
        {
            printf("Iteration: %ld\n", max_iters_limit);
            fflush(stdout);
        }
        max_iters_limit += iters_display_step;
        black_hole_solver_set_max_iters_limit(solver, max_iters_limit);
    } while (solver_ret_code == BLACK_HOLE_SOLVER__OUT_OF_ITERS);

    if (!solver_ret_code)
    {
        int col_idx, card_rank, card_suit;
        int next_move_ret_code;

        printf("%s\n", "Solved!");

        out_board(solver, display_boards);

        while ((next_move_ret_code = black_hole_solver_get_next_move(
            solver,
            &col_idx,
            &card_rank,
            &card_suit
            )) == BLACK_HOLE_SOLVER__SUCCESS)
        {
            printf ("Move a card from stack %d to the foundations\n\n"
                "Info: Card moved is %c%c\n\n\n====================\n\n",
                col_idx,
                (("0A23456789TJQK")[card_rank]), ("HCDS")[card_suit]
            );

            out_board(solver, display_boards);
        }

        if (next_move_ret_code != BLACK_HOLE_SOLVER__END)
        {
            fprintf(
                stderr,
                "%s - %d\n",
                "Get next move routine returned the wrong error code.",
                next_move_ret_code
            );
            ret = -1;
        }
    }
    else if (solver_ret_code == BLACK_HOLE_SOLVER__OUT_OF_ITERS)
    {
        printf("%s\n", "Intractable!");
        ret = -2;
    }
    else
    {
        printf("%s\n", "Unsolved!");
        ret = -1;
    }

    printf("\n\n--------------------\n"
        "Total number of states checked is %ld.\n"
        "This scan generated %ld states.\n",
        black_hole_solver_get_iterations_num(solver),
        black_hole_solver_get_num_states_in_collection(solver)
    );

    black_hole_solver_free(solver);

    return ret;
}
