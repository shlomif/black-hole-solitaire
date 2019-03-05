// main_common.h
// Copyright (C) 2018 Shlomi Fish <shlomif@cpan.org>
//
// Distributed under terms of the Expat license.
#include "solver_common.h"
#include "solver_run.h"

int main(int argc, char *argv[])
{
    unsigned long iters_display_step = 0;
    enum GAME_TYPE game_type = GAME__UNKNOWN;
    bool display_boards = FALSE;
    bool is_rank_reachability_prune_enabled = FALSE;
    bool place_queens_on_kings = FALSE;
    bool quiet_output = FALSE;
    bool wrap_ranks = TRUE;
    unsigned long max_iters_limit = ULONG_MAX;

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
        else if (!strcmp(argv[arg_idx], "--max-iters"))
        {
            if (argc == ++arg_idx)
            {
                fputs("Error! --max-iters requires an argument.\n", stderr);
                exit(-1);
            }
            max_iters_limit = (unsigned long)atol(argv[arg_idx++]);
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
                game_type = GAME__BH;
            }
            else if (!strcmp(g, "golf"))
            {
                game_type = GAME__GOLF;
                wrap_ranks = FALSE;
            }
            else if (!strcmp(g, "all_in_a_row"))
            {
                game_type = GAME__ALL;
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
            place_queens_on_kings = FALSE;
        }
        else if (!strcmp(argv[arg_idx], "--queens-on-kings"))
        {
            ++arg_idx;
            place_queens_on_kings = TRUE;
        }
        else if (!strcmp(argv[arg_idx], "--no-wrap-ranks"))
        {
            ++arg_idx;
            wrap_ranks = FALSE;
        }
        else if (!strcmp(argv[arg_idx], "--quiet"))
        {
            ++arg_idx;
            quiet_output = TRUE;
        }
        else if (!strcmp(argv[arg_idx], "--wrap-ranks"))
        {
            ++arg_idx;
            wrap_ranks = TRUE;
        }
        else if (!strcmp(argv[arg_idx], "--display-boards"))
        {
            ++arg_idx;
            display_boards = TRUE;
        }
        else if (!strcmp(argv[arg_idx], "--rank-reach-prune"))
        {
            ++arg_idx;
            is_rank_reachability_prune_enabled = TRUE;
        }
        else if (!strcmp(argv[arg_idx], "--iters-display-step"))
        {
            if (argc == ++arg_idx)
            {
                fprintf(stderr,
                    "Error! --iters-display-step requires an arguments.\n");
                exit(-1);
            }
            iters_display_step = (unsigned long)atol(argv[arg_idx++]);
        }
        else
        {
            break;
        }
    }

    for (; arg_idx < argc; ++arg_idx)
    {
        char *const filename = argv[arg_idx];
        fprintf(stdout, "[= Starting file %s =]\n", filename);
        solve_filename(filename, game_type, max_iters_limit, iters_display_step,
            display_boards, is_rank_reachability_prune_enabled,
            place_queens_on_kings, quiet_output, wrap_ranks);
        fprintf(stdout, "[= END of file %s =]\n", filename);
    }

    return 0;
}
