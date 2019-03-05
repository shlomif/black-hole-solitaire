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
    int arg_idx;
    const bhs_settings settings = parse_cmd_line(argc, argv, &arg_idx);

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
