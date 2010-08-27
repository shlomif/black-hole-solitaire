#ifndef BLACK_HOLE_SOLVER__H
#define BLACK_HOLE_SOLVER__H

enum
{
    BLACK_HOLE_SOLVER__SUCCESS = 0,
    BLACK_HOLE_SOLVER__OUT_OF_MEMORY,
    BLACK_HOLE_SOLVER__FOUNDATIONS_NOT_FOUND_AT_START,
    BLACK_HOLE_SOLVER__UNKNOWN_RANK,
    BLACK_HOLE_SOLVER__UNKNOWN_SUIT,
    BLACK_HOLE_SOLVER__TRAILING_CHARS,
    BLACK_HOLE_SOLVER__NOT_ENOUGH_COLUMNS,
    BLACK_HOLE_SOLVER__TOO_MANY_CARDS
};

typedef struct
{
    char nothing;
} black_hole_solver_instance_t;

extern int black_hole_solver_create(
    black_hole_solver_instance_t * * ret_instance
);

extern int black_hole_solver_read_board(
    black_hole_solver_instance_t * ret_instance,
    const char * board_string,
    int * error_line_number
);

#endif /* BLACK_HOLE_SOLVER__H */
