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
 * black_hole_solver.c - a solver for Black Hole Solitaire.
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <ctype.h>

#include "config.h"
#include "black_hole_solver.h"
#include "state.h"
#include "bit_rw.h"

#if (BHS_STATE_STORAGE == BHS_STATE_STORAGE_TOKYO_CAB_HASH)
#include "tokyo_cab_hash.h"
#elif (BHS_STATE_STORAGE == BHS_STATE_STORAGE_INTERNAL_HASH)
#include "fcs_hash.h"
#elif (BHS_STATE_STORAGE == BHS_STATE_STORAGE_GOOGLE_SPARSE_HASH)
#include "google_hash.h"
#else
#error Unknown state storage.
#endif

#include "alloc.h"

typedef struct
{
    unsigned char heights[BHS__MAX_NUM_COLUMNS];
    signed char foundations;
} bhs_unpacked_state_t;

typedef struct
{
    bhs_state_key_value_pair_t packed;
    bhs_unpacked_state_t unpacked;
} bhs_queue_item_t;

typedef bhs_queue_item_t bhs_solution_state_t;

typedef struct
{
    /*
     * TODO : rename from board_values.
     *
     * This is the ranks of the cards in the columns. It remains constant
     * for the duration of the game.
     * */
    bhs_rank_t board_values[BHS__MAX_NUM_COLUMNS][BHS__MAX_NUM_CARDS_IN_COL];

    bhs_rank_t initial_foundation;

    bh_solve_hash_t positions;

    bhs_card_string_t initial_foundation_string;
    bhs_card_string_t initial_board_card_strings[BHS__MAX_NUM_COLUMNS][BHS__MAX_NUM_CARDS_IN_COL];
    int initial_lens[BHS__MAX_NUM_COLUMNS];

    bhs_state_key_value_pair_t init_state;
    bhs_state_key_value_pair_t final_state;

    bhs_solution_state_t * states_in_solution;
    int num_states_in_solution, current_state_in_solution_idx;

    long iterations_num, num_states_in_collection, max_iters_limit;

    int num_columns;

    bhs_queue_item_t * queue;
    int queue_len, queue_max_len;
} bhs_solver_t;

int DLLEXPORT black_hole_solver_create(
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
        ret->states_in_solution = NULL;
        ret->iterations_num = 0;
        ret->num_states_in_collection = 0;
        ret->max_iters_limit = -1;

        bh_solve_hash_init(&(ret->positions));

        *ret_instance = (black_hole_solver_instance_t *)ret;

        return BLACK_HOLE_SOLVER__SUCCESS;
    }
}

#define MAX_RANK 12

static int parse_card(
    const char * * s,
    bhs_rank_t * foundation,
    bhs_card_string_t card
)
{
    strncpy(card, (*s), 2);
    card[2] = '\0';

    switch(*(*s))
    {
        case 'A':
            *foundation = 0;
            break;

        case '2':
            *foundation = 1;
            break;

        case '3':
            *foundation = 2;
            break;

        case '4':
            *foundation = 3;
            break;

        case '5':
            *foundation = 4;
            break;

        case '6':
            *foundation = 5;
            break;

        case '7':
            *foundation = 6;
            break;

        case '8':
            *foundation = 7;
            break;

        case '9':
            *foundation = 8;
            break;

        case 'T':
            *foundation = 9;
            break;

        case 'J':
            *foundation = 10;
            break;

        case 'Q':
            *foundation = 11;
            break;

        case 'K':
            *foundation = 12;
            break;

        default:
            return BLACK_HOLE_SOLVER__UNKNOWN_RANK;
    }

    (*s)++;

    switch (*(*s))
    {
        case 'H':
        case 'S':
        case 'D':
        case 'C':
            break;
        default:
            return BLACK_HOLE_SOLVER__UNKNOWN_SUIT;
    }
    (*s)++;

    return BLACK_HOLE_SOLVER__SUCCESS;
}


extern int DLLEXPORT black_hole_solver_read_board(
    black_hole_solver_instance_t * instance_proto,
    const char * board_string,
    int * error_line_number,
    int num_columns,
    int max_num_cards_in_col
)
{
    const char * s, * match;
    bhs_solver_t * solver;
    int ret_code, col_idx;

    solver = (bhs_solver_t *)instance_proto;

    solver->num_columns = num_columns;

    s = board_string;

    /* Read the foundations. */

    while ((*s) == '\n')
    {
        s++;
    }

    match = "Foundations: ";
    if (!strcmp(s, match))
    {
        *error_line_number = 1;
        return BLACK_HOLE_SOLVER__FOUNDATIONS_NOT_FOUND_AT_START;
    }

    s += strlen(match);

    while (isspace(*s) && ((*s) != '\n'))
    {
        s++;
    }

    if ((*s) == '-')
    {
        /* A non-initialized foundation. */
        solver->initial_foundation_string[0] = '\0';
        solver->initial_foundation = -1;
        s++;
    }
    else
    {
        ret_code =
            parse_card(&s,
                    &(solver->initial_foundation),
                    solver->initial_foundation_string
                    );
        if (ret_code)
        {
            *error_line_number = 1;
            return ret_code;
        }
    }

    if (*(s++) != '\n')
    {
        *error_line_number = 1;
        return BLACK_HOLE_SOLVER__TRAILING_CHARS;
    }

    for(col_idx = 0; col_idx < num_columns; col_idx++)
    {
        int pos_idx = 0;
        while ((*s != '\n') && (*s != '\0'))
        {
            if (pos_idx == max_num_cards_in_col)
            {
                *error_line_number = 2+col_idx;
                return BLACK_HOLE_SOLVER__TOO_MANY_CARDS;
            }

            ret_code =
                parse_card(&s,
                    &(solver->board_values[col_idx][pos_idx]),
                    solver->initial_board_card_strings[col_idx][pos_idx]
                );

            if (ret_code)
            {
                *error_line_number = 2+col_idx;
                return ret_code;
            }

            while ((*s) == ' ')
            {
                s++;
            }

            pos_idx++;
        }

        solver->initial_lens[col_idx] = pos_idx;

        if (*s == '\0')
        {
            *error_line_number = 2+col_idx;
            return BLACK_HOLE_SOLVER__NOT_ENOUGH_COLUMNS;
        }
        else
        {
            s++;
        }
    }

    return BLACK_HOLE_SOLVER__SUCCESS;
}

DLLEXPORT extern int black_hole_solver_set_max_iters_limit(
    black_hole_solver_instance_t * instance_proto,
    long limit
)
{
    bhs_solver_t * solver;

    solver = (bhs_solver_t *)instance_proto;
    solver->max_iters_limit = limit;

    return BLACK_HOLE_SOLVER__SUCCESS;
}

static GCC_INLINE void queue_item_populate_packed(
    bhs_solver_t * solver,
    bhs_queue_item_t * queue_item
)
{
    fc_solve_bit_writer_t bit_w;
    int col;
    int num_columns = solver->num_columns;

    queue_item->packed.key.foundations = queue_item->unpacked.foundations;

    fc_solve_bit_writer_init(&bit_w, queue_item->packed.key.data);

    for (col = 0; col < num_columns ; col++)
    {
        fc_solve_bit_writer_write(
            &bit_w,
            BHS__ALL_IN_A_ROW__BITS_PER_COL,
            queue_item->unpacked.heights[col]
        );
    }
}

static GCC_INLINE void queue_item_unpack(
    bhs_solver_t * solver,
    bhs_queue_item_t * queue_item
)
{
    fc_solve_bit_reader_t bit_r;
    int col;
    int num_columns = solver->num_columns;

    queue_item->unpacked.foundations = queue_item->packed.key.foundations;

    fc_solve_bit_reader_init(&bit_r, queue_item->packed.key.data);

    for (col = 0; col < num_columns ; col++)
    {
        queue_item->unpacked.heights[col] =
            fc_solve_bit_reader_read(
                &bit_r,
                BHS__ALL_IN_A_ROW__BITS_PER_COL
            );
    }
}

static void GCC_INLINE perform_move(
    bhs_solver_t * solver,
    bhs_rank_t card,
    int col_idx,
    bhs_queue_item_t * queue_item_copy_ptr
)
{
    bhs_unpacked_state_t next_state;

    next_state = queue_item_copy_ptr->unpacked;
    next_state.foundations = card;
    next_state.heights[col_idx]--;

    bhs_queue_item_t next_queue_item;

    next_queue_item.unpacked = next_state;
    memset(&(next_queue_item.packed), '\0', sizeof(next_queue_item.packed));

    next_queue_item.packed.value.parent_state = queue_item_copy_ptr->packed.key;
    next_queue_item.packed.value.col_idx = col_idx;

    queue_item_populate_packed(
        solver,
        &(next_queue_item)
    );

    if (! bh_solve_hash_insert(
            &(solver->positions),
            &(next_queue_item.packed)
    )
    )
    {
        solver->num_states_in_collection++;
        /* It's a new state - put it in the queue. */
        solver->queue[(solver->queue_len)++] = next_queue_item;

        if (solver->queue_len == solver->queue_max_len)
        {
            solver->queue = realloc(
                solver->queue,
                sizeof(solver->queue[0]) * (solver->queue_max_len += 64)
            );
        }
    }
}

extern int DLLEXPORT black_hole_solver_run(
    black_hole_solver_instance_t * ret_instance
)
{
    bhs_solver_t * solver;
    bhs_state_key_value_pair_t * init_state;
    bhs_unpacked_state_t state;

    bhs_queue_item_t * new_queue_item, queue_item_copy;
    int foundations;
    fcs_bool_t no_cards;
    int col_idx, pos;
    bhs_rank_t card;
    long iterations_num;
    long max_iters_limit;
    int num_columns;
    int i;

    solver = (bhs_solver_t *)ret_instance;

    num_columns = solver->num_columns;

    init_state = &(solver->init_state);

    max_iters_limit = solver->max_iters_limit;

    if (max_iters_limit < 0)
    {
        max_iters_limit = LONG_MAX;
    }

    solver->queue_max_len = 64;

    solver->queue = malloc(sizeof(solver->queue[0]) * solver->queue_max_len);

    solver->queue_len = 0;

    new_queue_item = &(solver->queue[solver->queue_len]);

    /* Populate the unpacked state. */
    for (i = 0 ; i < num_columns ; i++)
    {
        new_queue_item->unpacked.heights[i] = solver->initial_lens[i];
    }
    new_queue_item->unpacked.foundations = solver->initial_foundation;

    /* Populate the packed item from the unpacked one. */
    memset(&(new_queue_item->packed), '\0', sizeof(new_queue_item->packed));

    queue_item_populate_packed( solver, new_queue_item );

    *init_state = new_queue_item->packed;
    solver->queue_len++;

    solver->num_states_in_collection = 0;
    iterations_num = 0;

    bh_solve_hash_insert(
        &(solver->positions),
        init_state
    );

    solver->num_states_in_collection++;
    while (solver->queue_len > 0)
    {
        solver->queue_len--;
        queue_item_copy = solver->queue[solver->queue_len];
        state = queue_item_copy.unpacked;

        iterations_num++;

        foundations = state.foundations;

        no_cards = TRUE;

        for (col_idx = 0 ; col_idx < num_columns ; col_idx++)
        {
            if ( (pos = state.heights[col_idx]) )
            {
                no_cards = FALSE;
                card = solver->board_values[col_idx][pos-1];

                if ( (foundations == -1)
                        ||
                     (abs(card-foundations)%(MAX_RANK-1) == 1)
                )
                {
                    perform_move(
                        solver,
                        card,
                        col_idx,
                        &queue_item_copy
                    );
                }
            }
        }

        if (no_cards)
        {
            solver->final_state = queue_item_copy.packed;

            solver->iterations_num = iterations_num;

            free(solver->queue);
            solver->queue = NULL;

            return BLACK_HOLE_SOLVER__SUCCESS;
        }
        else
        {
            if (iterations_num == max_iters_limit)
            {
                solver->iterations_num = iterations_num;

                free(solver->queue);
                solver->queue = NULL;

                return BLACK_HOLE_SOLVER__OUT_OF_ITERS;
            }
        }
    }

    solver->iterations_num = iterations_num;

    free(solver->queue);
    solver->queue = NULL;

    return BLACK_HOLE_SOLVER__NOT_SOLVABLE;
}

extern int DLLEXPORT black_hole_solver_free(
    black_hole_solver_instance_t * instance_proto
)
{
    bhs_solver_t * solver;

    solver = (bhs_solver_t *)instance_proto;

    bh_solve_hash_free(&(solver->positions));

    free(solver);

    return BLACK_HOLE_SOLVER__SUCCESS;
}

static int suit_char_to_index(char suit)
{
    switch (suit)
    {
        case 'H':
            return 0;
        case 'C':
            return 1;
        case 'D':
            return 2;
        case 'S':
            return 3;
        default:
            return -1;
    }
}

DLLEXPORT extern int black_hole_solver_get_next_move(
    black_hole_solver_instance_t * instance_proto,
    int * col_idx_ptr,
    int * card_rank_ptr,
    int * card_suit_ptr /*  H=0, C=1, D=2, S=3 */
)
{
    bhs_solver_t * solver;

    solver = (bhs_solver_t *)instance_proto;

    if (! solver->states_in_solution)
    {
        bhs_solution_state_t * states;
        bhs_state_key_value_pair_t * key_ptr;
        bhs_solution_state_t temp_state;

        int i, num_states, max_num_states;

        num_states = 0;
        max_num_states = 53;

        states = malloc(sizeof(states[0]) * max_num_states);

        states[num_states].packed = (solver->final_state);
        queue_item_unpack(solver, &states[num_states]);

        while (memcmp(
            &(states[num_states].packed.key),
            &(solver->init_state.key),
            sizeof(states[num_states].packed.key)
        ))
        {
            if (num_states == max_num_states)
            {
                states =
                    realloc(
                        states,
                        sizeof(states[0]) * (max_num_states += 16)
                    );
            }

            key_ptr = (bhs_state_key_value_pair_t *)&(states[num_states].packed.value.parent_state);
            /* Look up the next state in the positions associative array. */
            bh_solve_hash_get(
                &(solver->positions),
                key_ptr,
                &(states[++num_states].packed)
            );
            queue_item_unpack(solver, &states[num_states]);
        }

        num_states++;

        /* Reverse the list in place. */
        for (i = 0 ; i < (num_states >> 1) ; i++)
        {
            temp_state = states[i];
            states[i] = states[num_states-1-i];
            states[num_states-1-i] = temp_state;
        }

        solver->states_in_solution = states;
        solver->num_states_in_solution = num_states;
        solver->current_state_in_solution_idx = 0;
    }

    if (solver->current_state_in_solution_idx == solver->num_states_in_solution-1)
    {
        *col_idx_ptr = *card_rank_ptr = *card_suit_ptr = -1;
        return BLACK_HOLE_SOLVER__END;
    }

    {
        bhs_solution_state_t next_state = solver->states_in_solution[
            ++solver->current_state_in_solution_idx
            ];

        int col_idx = next_state.packed.value.col_idx;
        int height = next_state.unpacked.heights[col_idx];

        *col_idx_ptr = col_idx;
        *card_rank_ptr = solver->board_values[col_idx][height]+1;
        *card_suit_ptr = suit_char_to_index(
            solver->initial_board_card_strings[col_idx][height][1]
        );

        return BLACK_HOLE_SOLVER__SUCCESS;
    }
}

DLLEXPORT extern long black_hole_solver_get_num_states_in_collection(
    black_hole_solver_instance_t * instance_proto
)
{
    return ((bhs_solver_t *)instance_proto)->num_states_in_collection;
}

DLLEXPORT extern long black_hole_solver_get_iterations_num(
    black_hole_solver_instance_t * instance_proto
)
{
    return ((bhs_solver_t *)instance_proto)->iterations_num;
}

