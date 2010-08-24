#include <stdlib.h>

#include "black_hole_solver.h"

typedef char bhs_rank_t;

#define MAX_NUM_COLUMNS 17
typedef struct 
{
    /* 
     * TODO : rename from board_values.
     *
     * This is the ranks of the cards in the columns. It remains constant
     * for the duration of the game.
     * */
    bhs_rank_t board_values[MAX_NUM_COLUMNS][3];

} bhs_solver_t;

/* We allocate 2-bits for the length of every column */
#define NUM_DATA_CHARS (MAX_NUM_COLUMNS * 2 / 8)
typedef struct
{
    char data[NUM_DATA_CHARS];
    bhs_rank_t foundations;
} bhs_state_key_t;

typedef char bhs_col_idx_t;

typedef struct
{
    /* The state from which this state was derived. */
    bhs_state_key_t parent_state;
    /* The index of the column that was changed. */
    bhs_col_idx_t col_idx;
} bhs_state_value_t;

int black_hole_solver_create(
    black_hole_solver_instance_t * * ret_instance
)
{
    bhs_solver_t * ret;

    ret = (bhs_solver_t *)malloc(sizeof(*ret));

    if (! ret)
    {
        *ret_instance =  NULL;
        return BLACK_HOLE_SOLVER__OUT_OF_MEMORY;
    }
    else
    {
        *ret_instance = (black_hole_solver_instance_t *)ret;
        return BLACK_HOLE_SOLVER__SUCCESS;
    }
}

static int run(const char * board_s)
{
}
