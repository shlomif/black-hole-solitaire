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

    bhs_state_key_value_pair_t * states_in_solution;
    int num_states_in_solution, current_state_in_solution_idx;

    long iterations_num, num_states_in_collection, max_iters_limit;

    int num_columns;
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

static GCC_INLINE void queue_item_populate_packed(
    bhs_queue_item_t * queue_item,
    int num_columns
)
{
    int two_cols_idx;
    int two_cols_offset;

    int cols_idx_limit =
    (
        (num_columns / BHS__ALL_IN_A_ROW__COLS_PER_BYTE)
            +
        (
            (num_columns % BHS__ALL_IN_A_ROW__COLS_PER_BYTE ) ? 1 : 0
        )
        - 1
    );

    queue_item->packed.key.foundations = queue_item->unpacked.foundations;

    for (two_cols_idx = 0, two_cols_offset = 0;
        two_cols_idx < cols_idx_limit ;
        two_cols_idx++, two_cols_offset += BHS__ALL_IN_A_ROW__COLS_PER_BYTE)
    {
        queue_item->packed.key.data[two_cols_idx] =
            (unsigned char)
            (
              (queue_item->unpacked.heights[two_cols_offset])
            | (queue_item->unpacked.heights[two_cols_offset+1] << BHS__ALL_IN_A_ROW__BITS_PER_COL)
            )
            ;
    }
    /* Only one left. */
    queue_item->packed.key.data[two_cols_idx] = (unsigned char)(queue_item->unpacked.heights[two_cols_offset]);
}

static void GCC_INLINE foobar(
    bhs_solver_t * solver,
    bhs_unpacked_state_t * next_state_ptr,
    bhs_unpacked_state_t * state_ptr,
    bhs_rank_t card,
    int col_idx,
    bhs_queue_item_t * queue_item_copy_ptr,
    int num_columns,
    long * num_states_in_collection_ptr,
    bhs_queue_item_t * * queue_ptr,
    int * queue_len_ptr,
    int * queue_max_len_ptr
)
{
    *next_state_ptr = *state_ptr;
    next_state_ptr->foundations = card;
    next_state_ptr->heights[col_idx]--;

    bhs_queue_item_t next_queue_item;

    next_queue_item.unpacked = *next_state_ptr;
    memset(&(next_queue_item.packed), '\0', sizeof(next_queue_item.packed));

    next_queue_item.packed.value.parent_state = queue_item_copy_ptr->packed.key;
    next_queue_item.packed.value.col_idx = col_idx;

    queue_item_populate_packed(
        &(next_queue_item),
        num_columns
    );

    if (! bh_solve_hash_insert(
            &(solver->positions),
            &(next_queue_item.packed)
    )
    )
    {
        (*num_states_in_collection_ptr)++;
        /* It's a new state - put it in the queue. */
        (*queue_ptr)[(*queue_len_ptr)++] = next_queue_item;

        if ((*queue_len_ptr) == (*queue_max_len_ptr))
        {
            (*queue_ptr) = realloc(
                (*queue_ptr),
                sizeof((*queue_ptr)[0]) * ((*queue_max_len_ptr) += 64)
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
    bhs_unpacked_state_t state, next_state;

    bhs_queue_item_t * queue, * new_queue_item, queue_item_copy;
    int queue_len, queue_max_len;
    int foundations;
    fcs_bool_t no_cards;
    int col_idx, pos;
    bhs_rank_t card;
    long iterations_num, num_states_in_collection;
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

    queue_max_len = 64;

    queue = malloc(sizeof(queue[0]) * queue_max_len);

    queue_len = 0;

    new_queue_item = &(queue[queue_len]);

    /* Populate the unpacked state. */
    for (i = 0 ; i < num_columns ; i++)
    {
        new_queue_item->unpacked.heights[i] = solver->initial_lens[i];
    }
    new_queue_item->unpacked.foundations = solver->initial_foundation;

    /* Populate the packed item from the unpacked one. */
    memset(&(new_queue_item->packed), '\0', sizeof(new_queue_item->packed));

    queue_item_populate_packed( new_queue_item, num_columns );

    *init_state = new_queue_item->packed;
    queue_len++;

    num_states_in_collection = 0;
    iterations_num = 0;

    bh_solve_hash_insert(
        &(solver->positions),
        init_state
    );

    num_states_in_collection++;
    while (queue_len > 0)
    {
        queue_len--;
        queue_item_copy = queue[queue_len];
        state = queue_item_copy.unpacked;

        iterations_num++;

        foundations = state.foundations;

        no_cards = TRUE;

        if (foundations == -1)
        {
            for (col_idx = 0 ; col_idx < num_columns ; col_idx++)
            {
#define BYTE_POS() (col_idx >> 1)
#define BIT_OFFSET() ((col_idx&(2-1))<<2)
                if ((pos = state.heights[col_idx]))
                {
                    no_cards = FALSE;
                    card = solver->board_values[col_idx][pos-1];

#define CALL_foobar() \
                    foobar( \
                        solver, \
                        &next_state, \
                        &state, \
                        card, \
                        col_idx, \
                        &queue_item_copy, \
                        num_columns, \
                        &num_states_in_collection, \
                        &queue, \
                        &queue_len, \
                        &queue_max_len \
                    )

                    CALL_foobar();

                }
            }
        }
        else
        {
            for (col_idx = 0 ; col_idx < num_columns ; col_idx++)
            {
                if ( (pos = state.heights[col_idx] ) )
                {
                    no_cards = FALSE;

                    card = solver->board_values[col_idx][pos-1];

                    if (abs(card-foundations)%(MAX_RANK-1) == 1)
                    {
                        CALL_foobar();
                    }
                }
            }
        }

        if (no_cards)
        {
            solver->final_state = queue_item_copy.packed;

            solver->iterations_num = iterations_num;
            solver->num_states_in_collection = num_states_in_collection;

            free(queue);

            return BLACK_HOLE_SOLVER__SUCCESS;
        }
        else
        {
            if (iterations_num == max_iters_limit)
            {
                solver->iterations_num = iterations_num;
                solver->num_states_in_collection = num_states_in_collection;

                free(queue);

                return BLACK_HOLE_SOLVER__OUT_OF_ITERS;
            }
        }
    }

    solver->iterations_num = iterations_num;
    solver->num_states_in_collection = num_states_in_collection;

    free(queue);

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
        bhs_state_key_value_pair_t * states;
        bhs_state_key_value_pair_t * key_ptr;
        bhs_state_key_value_pair_t temp_state;

        int i, num_states, max_num_states;

        num_states = 0;
        max_num_states = 53;

        states = malloc(sizeof(states[0]) * max_num_states);

        states[num_states] = (solver->final_state);

        while (memcmp(
            &(states[num_states].key),
            &(solver->init_state.key),
            sizeof(states[num_states].key)
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

            key_ptr = (bhs_state_key_value_pair_t *)&(states[num_states].value.parent_state);
            /* Look up the next state in the positions associative array. */
            bh_solve_hash_get(
                &(solver->positions),
                key_ptr,
                states+(++num_states)
            );
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
        bhs_state_key_value_pair_t next_state;
        int height;

        next_state = solver->states_in_solution[
            ++solver->current_state_in_solution_idx
            ];

        *col_idx_ptr = next_state.value.col_idx;
        height =
        (
        (
            (next_state.key.data[(*col_idx_ptr)>>1]
                >>
            (((*col_idx_ptr)&(2-1)) << 2)) & 0xF
        )
        );

        (*card_rank_ptr) = solver->board_values[*col_idx_ptr][height]+1;
        (*card_suit_ptr) = suit_char_to_index(solver->initial_board_card_strings[*col_idx_ptr][height][1]);

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

