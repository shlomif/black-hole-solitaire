#ifndef BLACK_HOLE_SOLVER__H
#define BLACK_HOLE_SOLVER__H

enum
{
    BLACK_HOLE_SOLVER__SUCCESS = 0,
    BLACK_HOLE_SOLVER__OUT_OF_MEMORY = 1,
};

typedef struct
{
    char nothing;
} black_hole_solver_instance_t;

extern int black_hole_solver_create(
    black_hole_solver_instance_t * * ret_instance
);

#endif /* BLACK_HOLE_SOLVER__H */
