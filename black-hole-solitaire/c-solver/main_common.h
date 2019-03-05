// main_common.h
// Copyright (C) 2018 Shlomi Fish <shlomif@cpan.org>
//
// Distributed under terms of the Expat license.
#pragma once
#include "solver_common.h"

static inline int solver_run(black_hole_solver_instance_t *const solver,
    const unsigned long max_iters_limit,
    const unsigned long iters_display_step);

int main(int argc, char *argv[])
{
    bhs_settings settings;
    settings.iters_display_step = 0;
    settings.game_type = GAME__UNKNOWN;
    settings.display_boards = FALSE;
    settings.is_rank_reachability_prune_enabled = FALSE;
    settings.place_queens_on_kings = FALSE;
    settings.quiet_output = FALSE;
    settings.wrap_ranks = TRUE;
    settings.max_iters_limit = ULONG_MAX;

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
                settings.wrap_ranks = FALSE;
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
            settings.place_queens_on_kings = FALSE;
        }
        else if (!strcmp(argv[arg_idx], "--queens-on-kings"))
        {
            ++arg_idx;
            settings.place_queens_on_kings = TRUE;
        }
        else if (!strcmp(argv[arg_idx], "--no-wrap-ranks"))
        {
            ++arg_idx;
            settings.wrap_ranks = FALSE;
        }
        else if (!strcmp(argv[arg_idx], "--quiet"))
        {
            ++arg_idx;
            settings.quiet_output = TRUE;
        }
        else if (!strcmp(argv[arg_idx], "--wrap-ranks"))
        {
            ++arg_idx;
            settings.wrap_ranks = TRUE;
        }
        else if (!strcmp(argv[arg_idx], "--display-boards"))
        {
            ++arg_idx;
            settings.display_boards = TRUE;
        }
        else if (!strcmp(argv[arg_idx], "--rank-reach-prune"))
        {
            ++arg_idx;
            settings.is_rank_reachability_prune_enabled = TRUE;
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

    char *filename = NULL;
    if (argc > arg_idx)
    {
        if (strcmp(argv[arg_idx], "-"))
        {
            filename = argv[arg_idx];
        }
        ++arg_idx;
    }

    return solve_filename(filename, settings);
}
