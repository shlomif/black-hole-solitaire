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
 * black_hole_solver.h - a solver for Black Hole Solitaire - header of the API.
 */

#ifndef BLACK_HOLE_SOLVER__H
#define BLACK_HOLE_SOLVER__H


#ifdef __cplusplus
extern "C" {
#endif

#ifdef _MSC_VER
  #ifdef BUILDING_DLL
    #define DLLEXPORT __declspec(dllexport)
  #else
    #define DLLEXPORT __declspec(dllimport)
  #endif
  #define DLLLOCAL
#elif defined(__GNUC__)
    #define DLLEXPORT __attribute__ ((visibility("default")))
    #define DLLLOCAL __attribute__ ((visibility("hidden")))
#else
    #define DLLEXPORT
    #define DLLLOCAL
#endif

enum
{
    BLACK_HOLE_SOLVER__SUCCESS = 0,
    BLACK_HOLE_SOLVER__OUT_OF_MEMORY,
    BLACK_HOLE_SOLVER__FOUNDATIONS_NOT_FOUND_AT_START,
    BLACK_HOLE_SOLVER__UNKNOWN_RANK,
    BLACK_HOLE_SOLVER__UNKNOWN_SUIT,
    BLACK_HOLE_SOLVER__TRAILING_CHARS,
    BLACK_HOLE_SOLVER__NOT_ENOUGH_COLUMNS,
    BLACK_HOLE_SOLVER__TOO_MANY_CARDS,
    BLACK_HOLE_SOLVER__NOT_SOLVABLE,
    BLACK_HOLE_SOLVER__END
};

typedef struct
{
    char nothing;
} black_hole_solver_instance_t;

DLLEXPORT extern int black_hole_solver_create(
    black_hole_solver_instance_t * * ret_instance
);

DLLEXPORT extern int black_hole_solver_read_board(
    black_hole_solver_instance_t * ret_instance,
    const char * board_string,
    int * error_line_number
);

DLLEXPORT extern int black_hole_solver_run(
    black_hole_solver_instance_t * ret_instance
);

DLLEXPORT extern int black_hole_solver_free(
    black_hole_solver_instance_t * instance_proto
);

DLLEXPORT extern int black_hole_solver_get_next_move(
    black_hole_solver_instance_t * instance_proto,
    int * col_idx_ptr,
    int * card_rank_ptr,
    int * card_suit_ptr /* Will return H=0, C=1, D=2, S=3 */
);

DLLEXPORT extern long black_hole_solver_get_num_states_in_collection(
    black_hole_solver_instance_t * instance_proto
);

DLLEXPORT extern long black_hole_solver_get_iterations_num(
    black_hole_solver_instance_t * instance_proto
);

#ifdef __cplusplus
}
#endif

#endif /* BLACK_HOLE_SOLVER__H */
