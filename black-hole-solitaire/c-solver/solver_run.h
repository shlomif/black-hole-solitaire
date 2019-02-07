// solver_run.h
// Copyright (C) 2018 Shlomi Fish <shlomif@cpan.org>
//
// Distributed under terms of the Expat license.
#pragma once

static inline int solver_run(black_hole_solver_instance_t *const solver,
    const unsigned long max_iters_limit, const unsigned long iters_display_step)
{
    unsigned long iters_limit = min(iters_display_step, max_iters_limit);
    black_hole_solver_set_max_iters_limit(solver, iters_limit);
    unsigned long iters_num;
    int solver_ret_code;

    do
    {
        solver_ret_code = black_hole_solver_run(solver);
        iters_num = black_hole_solver_get_iterations_num(solver);
        if (iters_limit == iters_num)
        {
            printf("Iteration: %lu\n", iters_limit);
            fflush(stdout);
        }
        iters_limit += iters_display_step;
        iters_limit = min(iters_limit, max_iters_limit);
        black_hole_solver_set_max_iters_limit(solver, iters_limit);
    } while ((solver_ret_code == BLACK_HOLE_SOLVER__OUT_OF_ITERS) &&
             (iters_num < max_iters_limit));
    return solver_ret_code;
}
