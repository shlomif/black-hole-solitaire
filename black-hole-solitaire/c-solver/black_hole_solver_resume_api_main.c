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
// black_hole_solver_resume_api_main.c - solver using the resume API

#include "main_common.h"

static inline int solver_run(black_hole_solver_instance_t *const solver,
    const long max_iters_limit, const long iters_display_step)
{
    long iters_limit = min(iters_display_step, max_iters_limit);
    black_hole_solver_set_max_iters_limit(solver, iters_limit);
    long iters_num;
    int solver_ret_code;

    do
    {
        solver_ret_code = black_hole_solver_run(solver);
        iters_num = black_hole_solver_get_iterations_num(solver);
        if (iters_limit == iters_num)
        {
            printf("Iteration: %ld\n", iters_limit);
            fflush(stdout);
        }
        iters_limit += iters_display_step;
        iters_limit = min(iters_limit, max_iters_limit);
        black_hole_solver_set_max_iters_limit(solver, iters_limit);
    } while ((solver_ret_code == BLACK_HOLE_SOLVER__OUT_OF_ITERS) &&
             (iters_num < max_iters_limit));
    return solver_ret_code;
}
