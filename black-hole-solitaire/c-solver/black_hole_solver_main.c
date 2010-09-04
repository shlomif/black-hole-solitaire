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

#include "black_hole_solver.h"

#define MAX_LEN_BOARD_STRING 4092

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

    if (black_hole_solver_create(&solver))
    {
        fprintf(stderr, "%s\n", "Could not initialise solver (out-of-memory)");
        exit(-1);
    }

    arg_idx = 1;
    if (argc > arg_idx)
    {
        if (!strcmp(argv[arg_idx], "--max-iters"))
        {
            arg_idx++;
            if (argc == arg_idx)
            {
                fprintf(stderr, "Error! --max-iters requires an arguments.\n");
                exit(-1);
            }
            max_iters_limit = atol(argv[arg_idx++]);
        }
    }

    black_hole_solver_set_max_iters_limit(solver, max_iters_limit);
    
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
        &error_line_num
        ))
    {
        fprintf(stderr, "Error reading the board at line No. %d!\n", error_line_num);
        exit(-1);
    }

    ret = 0;

    solver_ret_code = black_hole_solver_run(solver);
    if (!solver_ret_code)
    {
        int col_idx, card_rank, card_suit;
        int next_move_ret_code;

        printf("Solved!\n");

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
        printf("Intractable!\n");
        ret = -2;
    }
    else
    {
        printf("Unsolved!\n");
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
