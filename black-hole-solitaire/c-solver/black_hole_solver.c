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

#include "config.h"
#include "black_hole_solver.h"
#include "state.h"

#if (BHS_STATE_STORAGE == BHS_STATE_STORAGE_TOKYO_CAB_HASH)
#include "tokyo_cab_hash.h"
#else
#include "fcs_hash.h"
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
    bhs_rank_t board_values[MAX_NUM_COLUMNS][MAX_NUM_CARDS_IN_COL];

    bhs_rank_t initial_foundation;

    bhs_compact_allocator_t allocator;
    bh_solve_hash_t positions;

    bhs_card_string_t initial_foundation_string;
    bhs_card_string_t initial_board_card_strings[MAX_NUM_COLUMNS][MAX_NUM_CARDS_IN_COL];
    int initial_lens[MAX_NUM_COLUMNS];

    bhs_state_key_value_pair_t * init_state;
    bhs_state_key_value_pair_t * final_state;

    bhs_state_key_value_pair_t * states_in_solution;
    int num_states_in_solution, current_state_in_solution_idx;
    
    long iterations_num, num_states_in_collection;
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
        bh_solve_compact_allocator_init(&(ret->allocator));
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
    int * error_line_number
)
{
    const char * s, * match;
    bhs_solver_t * solver;
    int ret_code, col_idx;
    
    solver = (bhs_solver_t *)instance_proto;

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

    if (*(s++) != '\n')
    {
        *error_line_number = 1;
        return BLACK_HOLE_SOLVER__TRAILING_CHARS;
    }

    for(col_idx = 0; col_idx < MAX_NUM_COLUMNS; col_idx++)
    {
        int pos_idx = 0;
        while ((*s != '\n') && (*s != '\0'))
        {
            if (pos_idx == MAX_NUM_CARDS_IN_COL)
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


typedef  unsigned long  int  ub4;   /* unsigned 4-byte quantities */
typedef  unsigned       char ub1;

static GCC_INLINE ub4 perl_hash_function(
    register ub1 *s_ptr,        /* the key */
    register ub4  length        /* the length of the key */
    )
{
    register ub4  hash_value_int = 0;
    register ub1 * s_end = s_ptr+length;

    while (s_ptr < s_end)
    {
        hash_value_int += (hash_value_int << 5) + *(s_ptr++);
    }
    hash_value_int += (hash_value_int>>5);

    return hash_value_int;
}

extern int DLLEXPORT black_hole_solver_run(
    black_hole_solver_instance_t * ret_instance
)
{
    bhs_solver_t * solver;
    bhs_state_key_value_pair_t * init_state;
#if (! (BHS_STATE_STORAGE == BHS_STATE_STORAGE_TOKYO_CAB_HASH))
    void * init_state_existing;
#endif
    bhs_state_key_value_pair_t * state, * next_state;
    int four_cols_idx, four_cols_offset;
    bhs_state_key_value_pair_t * * queue;
    int queue_len, queue_max_len;
    int foundations;
    fcs_bool_t no_cards;
    int col_idx, pos;
    bhs_rank_t card;
    long iterations_num, num_states_in_collection;

    solver = (bhs_solver_t *)ret_instance;

    solver->init_state = init_state = 
        fcs_compact_alloc_ptr(&(solver->allocator), sizeof(*init_state));
    memset(init_state, '\0', sizeof(*init_state));
    init_state->key.foundations = solver->initial_foundation;

    for (four_cols_idx = 0, four_cols_offset = 0; four_cols_idx < 4; four_cols_idx++, four_cols_offset += 4)
    {
        init_state->key.data[four_cols_idx] =
            (unsigned char)
            (
              (solver->initial_lens[four_cols_offset]) 
            | (solver->initial_lens[four_cols_offset+1] << 2)    
            | (solver->initial_lens[four_cols_offset+2] << 4)    
            | (solver->initial_lens[four_cols_offset+3] << 6)    
            )
            ;
    }
    /* Only one left. */
    init_state->key.data[four_cols_idx] = (unsigned char)(solver->initial_lens[four_cols_offset]);

    num_states_in_collection = 0;
    iterations_num = 0;

    bh_solve_hash_insert(
        &(solver->positions),
        init_state
#if (! (BHS_STATE_STORAGE == BHS_STATE_STORAGE_TOKYO_CAB_HASH))
        , &init_state_existing
        , perl_hash_function(((ub1 *)&(init_state->key)), sizeof(init_state->key))
#endif
    );

    num_states_in_collection++;

    queue_max_len = 64;

    queue = malloc(sizeof(queue[0]) * queue_max_len);

    queue_len = 0;
    queue[queue_len++] = init_state;

    while (queue_len > 0)
    {
        state = queue[--queue_len];
        iterations_num++;

        foundations = state->key.foundations;

        no_cards = TRUE;

        for (col_idx = 0 ; col_idx < MAX_NUM_COLUMNS ; col_idx++)
        {
            if ((pos = (
                (state->key.data[(col_idx >> 2)] >> ((col_idx&(4-1))<<1))
                    &
                    (4-1)
                )
            ))
            {
                no_cards = FALSE;

                card = solver->board_values[col_idx][pos-1];
                
                if (abs(card-foundations)%(MAX_RANK-1) == 1)
                {
                    next_state = fcs_compact_alloc_ptr(
                            &(solver->allocator), 
                            sizeof(*next_state)
                            );
                    *next_state = *state;
                    next_state->key.foundations = card;
                    next_state->key.data[(col_idx>>2)] &= 
                        (~(0x3 << ((col_idx&0x3)<<1)));
                    next_state->key.data[(col_idx>>2)] |=
                        ((pos-1) << ((col_idx&0x3)<<1));
                    next_state->value.parent_state = state->key;
                    next_state->value.col_idx = col_idx;

                    if (! bh_solve_hash_insert(
                        &(solver->positions),
                        next_state
#if (! (BHS_STATE_STORAGE == BHS_STATE_STORAGE_TOKYO_CAB_HASH))
                        , &init_state_existing
                        , perl_hash_function(((ub1 *)&(next_state->key)), 
                            sizeof(next_state->key))
#endif
                        )
                    )
                    {
                        num_states_in_collection++;
                        /* It's a new state - put it in the queue. */
                        queue[queue_len++] = next_state;

                        if (queue_len == queue_max_len)
                        {
                            queue = realloc(
                                queue,
                                sizeof(queue[0]) * (queue_max_len += 64)
                            );
                        }
                    }
                    else
                    {
                        fcs_compact_alloc_release(&(solver->allocator));
                    }
                }
            }
        }

        if (no_cards)
        {
            solver->final_state = state;

            solver->iterations_num = iterations_num;
            solver->num_states_in_collection = num_states_in_collection;

            return BLACK_HOLE_SOLVER__SUCCESS;
        }
    }

    solver->iterations_num = iterations_num;
    solver->num_states_in_collection = num_states_in_collection;

    return BLACK_HOLE_SOLVER__NOT_SOLVABLE;
}

extern int DLLEXPORT black_hole_solver_free(
    black_hole_solver_instance_t * instance_proto
)
{
    bhs_solver_t * solver;

    solver = (bhs_solver_t *)instance_proto;

    bh_solve_compact_allocator_finish(&(solver->allocator));
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
        bhs_state_key_t * key_ptr;
        bhs_state_key_value_pair_t temp_state;

        int i, num_states, max_num_states;
#if (! (BHS_STATE_STORAGE == BHS_STATE_STORAGE_TOKYO_CAB_HASH))
        void * next_state;
#endif

        num_states = 0;
        max_num_states = 53;

        states = malloc(sizeof(states[0]) * max_num_states);
        
        states[num_states] = (*(solver->final_state));

        while (memcmp(
            &(states[num_states].key),
            &(solver->init_state->key),
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
         
            key_ptr = &(states[num_states].value.parent_state);
            /* Look up the next state in the positions associative array. */
#if (BHS_STATE_STORAGE == BHS_STATE_STORAGE_TOKYO_CAB_HASH)
            bh_solve_hash_get(
                &(solver->positions),
                key_ptr,
                states+(++num_states)
            );
#else
            bh_solve_hash_insert(
                &(solver->positions),
                key_ptr,
                &next_state,
                perl_hash_function((ub1 *)key_ptr, sizeof(*key_ptr))
            );

            memcpy(
                states+(++num_states),
                next_state, 
                sizeof(states[0])
            );            
#endif

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
            (next_state.key.data[(*col_idx_ptr)>>2]
                >>
            (((*col_idx_ptr)&0x3) << 1)) & 0x3
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

