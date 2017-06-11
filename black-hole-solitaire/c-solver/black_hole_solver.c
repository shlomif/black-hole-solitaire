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
#include <black-hole-solver/black_hole_solver.h>
#include "state.h"
#include "bit_rw.h"
#include "typeof_wrap.h"

#if (BHS_STATE_STORAGE == BHS_STATE_STORAGE_TOKYO_CAB_HASH)
#include "tokyo_cab_hash.h"
#elif (BHS_STATE_STORAGE == BHS_STATE_STORAGE_INTERNAL_HASH)
#include "fcs_hash.h"
#elif (BHS_STATE_STORAGE == BHS_STATE_STORAGE_GOOGLE_SPARSE_HASH)
#include "google_hash.h"
#else
#error Unknown state storage.
#endif

#include "rank_reach_prune.h"

#define NUM_SUITS 4
enum BHS_SUITS
{
    SUIT_H,
    SUIT_C,
    SUIT_D,
    SUIT_S,
    INVALID_SUIT = -1
};

static int suit_char_to_index(char suit)
{
    switch (suit)
    {
        case 'H':
            return SUIT_H;
        case 'C':
            return SUIT_C;
        case 'D':
            return SUIT_D;
        case 'S':
            return SUIT_S;
        default:
            return INVALID_SUIT;
    }
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
} bhs_solution_state_t;

typedef struct
{
    unsigned char c[NUM_RANKS];
} bhs_rank_counts_t;

typedef struct
{
    bhs_solution_state_t s;
    bhs_rank_counts_t rank_counts;
} bhs_queue_item_t;

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
    long iters_display_step;

    int num_columns;
    int bits_per_column;

    bhs_queue_item_t * queue;
    int queue_len, queue_max_len;
    int sol_foundations_card_rank, sol_foundations_card_suit;
    fcs_bool_t is_rank_reachability_prune_enabled;
    fcs_bool_t require_initialization;
} bhs_solver_t;

int DLLEXPORT black_hole_solver_create(
    black_hole_solver_instance_t * * ret_instance
)
{
    bhs_solver_t * const ret = (bhs_solver_t *)malloc(sizeof(*ret));

    if (! ret)
    {
        *ret_instance =  NULL;
        return BLACK_HOLE_SOLVER__OUT_OF_MEMORY;
    }
    ret->require_initialization = TRUE;
    ret->states_in_solution = NULL;
    ret->iterations_num = 0;
    ret->num_states_in_collection = 0;
    ret->max_iters_limit = -1;
    ret->is_rank_reachability_prune_enabled = FALSE;
    ret->iters_display_step = 0;
    ret->num_columns = 0;

    bh_solve_hash_init(&(ret->positions));

    *ret_instance = (black_hole_solver_instance_t *)ret;

    return BLACK_HOLE_SOLVER__SUCCESS;
}

DLLEXPORT extern int black_hole_solver_enable_rank_reachability_prune(
    black_hole_solver_instance_t * const instance_proto,
    const fcs_bool_t enabled_status
)
{
    bhs_solver_t * const solver = (bhs_solver_t *)instance_proto;
    solver->is_rank_reachability_prune_enabled = enabled_status ? TRUE : FALSE;

    return BLACK_HOLE_SOLVER__SUCCESS;
}

#define MAX_RANK (NUM_RANKS-1)

enum BHS_RANKS
{
    RANK_A,
    RANK_2,
    RANK_3,
    RANK_4,
    RANK_5,
    RANK_6,
    RANK_7,
    RANK_8,
    RANK_9,
    RANK_T,
    RANK_J,
    RANK_Q,
    RANK_K,
};

static int parse_card(
    const char * * s,
    bhs_rank_t * const foundation,
    bhs_card_string_t card,
    int * const suit_ptr
)
{
    strncpy(card, (*s), BHS_CARD_STRING_LEN);
    card[BHS_CARD_STRING_LEN] = '\0';

    /* Short for value. */
    bhs_rank_t v;

    switch(*(*s))
    {
        case 'A':
            v = RANK_A;
            break;

        case '2':
            v = RANK_2;
            break;

        case '3':
            v = RANK_3;
            break;

        case '4':
            v = RANK_4;
            break;

        case '5':
            v = RANK_5;
            break;

        case '6':
            v = RANK_6;
            break;

        case '7':
            v = RANK_7;
            break;

        case '8':
            v = RANK_8;
            break;

        case '9':
            v = RANK_9;
            break;

        case 'T':
            v = RANK_T;
            break;

        case 'J':
            v = RANK_J;
            break;

        case 'Q':
            v = RANK_Q;
            break;

        case 'K':
            v = RANK_K;
            break;

        default:
            return BLACK_HOLE_SOLVER__UNKNOWN_RANK;
    }

    *foundation = v;

    (*s)++;

    switch (*(*s))
    {
        case 'H':
        case 'S':
        case 'D':
        case 'C':
            if (suit_ptr)
            {
                *suit_ptr = suit_char_to_index(*(*(s)));
            }
            break;
        default:
            return BLACK_HOLE_SOLVER__UNKNOWN_SUIT;
    }
    (*s)++;

    return BLACK_HOLE_SOLVER__SUCCESS;
}

static inline fcs_bool_t string_find_prefix(
    const char * * s,
    const char * const prefix
)
{
    register size_t len = strlen(prefix);

    if (strncmp(*s, prefix, len) != 0)
    {
        return FALSE;
    }

    (*s) += len;

    return TRUE;
}

extern int DLLEXPORT black_hole_solver_read_board(
    black_hole_solver_instance_t * const instance_proto,
    const char * const board_string,
    int * const error_line_number,
    const int num_columns,
    const int max_num_cards_in_col,
    const int bits_per_column
)
{
    int line_num = 1;

    if (num_columns > BHS__MAX_NUM_COLUMNS)
    {
        return BLACK_HOLE_SOLVER__INVALID_INPUT;
    }

    bhs_solver_t * const solver = (bhs_solver_t *)instance_proto;

    solver->num_columns = num_columns;
    solver->bits_per_column = bits_per_column;

    const char * s = board_string;

    /* Read the foundations. */

    while ((*s) == '\n')
    {
        line_num++;
        s++;
    }

#define MYRET(code) \
    { \
        *error_line_number = line_num; \
        return code; \
    }

    if (! string_find_prefix(&s, "Foundations: "))
    {
        MYRET (BLACK_HOLE_SOLVER__FOUNDATIONS_NOT_FOUND_AT_START);
    }

    while (isspace(*s) && ((*s) != '\n'))
    {
        s++;
    }

    if ((*s) == '-')
    {
        /* A non-initialized foundation. */
        solver->initial_foundation_string[0] = '\0';
        solver->initial_foundation = -1;
        solver->sol_foundations_card_rank = -1;
        solver->sol_foundations_card_suit = -1;
        s++;
    }
    else
    {
        const int ret_code =
            parse_card(&s,
                    &(solver->initial_foundation),
                    solver->initial_foundation_string,
                    &(solver->sol_foundations_card_suit)
                    );

        solver->sol_foundations_card_rank = solver->initial_foundation;

        if (ret_code)
        {
            MYRET (ret_code);
        }
    }

    if (*(s++) != '\n')
    {
        MYRET (BLACK_HOLE_SOLVER__TRAILING_CHARS);
    }
    line_num++;

    for (int col_idx = 0; col_idx < num_columns; col_idx++, line_num++)
    {
        int pos_idx = 0;
        while ((*s != '\n') && (*s != '\0'))
        {
            if (pos_idx == max_num_cards_in_col)
            {
                MYRET (BLACK_HOLE_SOLVER__TOO_MANY_CARDS);
            }

            const int ret_code =
                parse_card(&s,
                    &(solver->board_values[col_idx][pos_idx]),
                    solver->initial_board_card_strings[col_idx][pos_idx],
                    NULL
                );

            if (ret_code)
            {
                MYRET (ret_code);
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
            MYRET ( BLACK_HOLE_SOLVER__NOT_ENOUGH_COLUMNS );
        }
        else
        {
            s++;
        }
    }

    *error_line_number = -1;
    return BLACK_HOLE_SOLVER__SUCCESS;
}

#undef MYRET


DLLEXPORT extern int black_hole_solver_set_max_iters_limit(
    black_hole_solver_instance_t * const instance_proto,
    const long limit
)
{
    ((bhs_solver_t * const)instance_proto)->max_iters_limit = limit;

    return BLACK_HOLE_SOLVER__SUCCESS;
}

DLLEXPORT extern int black_hole_solver_set_iters_display_step(
    black_hole_solver_instance_t * const instance_proto,
    const long iters_display_step
)
{
    if (iters_display_step < 0)
    {
        return BLACK_HOLE_SOLVER__INVALID_INPUT;
    }
    ((bhs_solver_t * const)instance_proto)->iters_display_step
        = iters_display_step;

    return BLACK_HOLE_SOLVER__SUCCESS;
}

static inline void queue_item_populate_packed(
    bhs_solver_t * const solver,
    bhs_queue_item_t * const queue_item
)
{
    queue_item->s.packed.key.foundations = queue_item->s.unpacked.foundations;

    fc_solve_bit_writer_t bit_w;
    fc_solve_bit_writer_init(&bit_w, queue_item->s.packed.key.data);

    const_SLOT(num_columns, solver);
    const_SLOT(bits_per_column, solver);
    for (int col = 0; col < num_columns ; col++)
    {
        fc_solve_bit_writer_write(
            &bit_w,
            bits_per_column,
            queue_item->s.unpacked.heights[col]
        );
    }
}

static inline void queue_item_unpack(
    bhs_solver_t * const solver,
    bhs_solution_state_t * const queue_item
)
{
    const_SLOT(num_columns, solver);
    const_SLOT(bits_per_column, solver);

    queue_item->unpacked.foundations = queue_item->packed.key.foundations;

    fc_solve_bit_reader_t bit_r;
    fc_solve_bit_reader_init(&bit_r, queue_item->packed.key.data);

    for (int col = 0; col < num_columns ; col++)
    {
        queue_item->unpacked.heights[col] =
            (typeof(queue_item->unpacked.heights[col]))fc_solve_bit_reader_read(
                &bit_r,
                bits_per_column
            );
    }
}

static inline void perform_move(
    bhs_solver_t * const solver,
    const bhs_rank_t card,
    const int col_idx,
    const bhs_queue_item_t * const queue_item_copy_ptr
)
{
    bhs_unpacked_state_t next_state
        = queue_item_copy_ptr->s.unpacked;

    next_state.foundations = card;
    next_state.heights[col_idx]--;

    bhs_queue_item_t next_queue_item;

    next_queue_item.s.unpacked = next_state;
    memset(&(next_queue_item.s.packed), '\0', sizeof(next_queue_item.s.packed));

    next_queue_item.s.packed.value.parent_state = queue_item_copy_ptr->s.packed.key;
    next_queue_item.s.packed.value.col_idx = (typeof(next_queue_item.s.packed.value.col_idx))col_idx;

    queue_item_populate_packed(
        solver,
        &(next_queue_item)
    );

    next_queue_item.rank_counts = queue_item_copy_ptr->rank_counts;
    next_queue_item.rank_counts.c[(ssize_t)card]--;

    if (! bh_solve_hash_insert(
            &(solver->positions),
            &(next_queue_item.s.packed)
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
                sizeof(solver->queue[0]) * (size_t)(solver->queue_max_len += 64)
            );
        }
    }
}

static inline long maxify(long n)
{
    return ((n < 0) ? LONG_MAX : n);
}

static inline bhs_state_key_value_pair_t setup_first_queue_item(
    bhs_solver_t * const solver
)
{
    const_SLOT(num_columns, solver);

    typeof(solver->queue[solver->queue_len]) * const new_queue_item =
        &(solver->queue[solver->queue_len]);

    /* Populate the unpacked state. */
    for (int i = 0 ; i < num_columns ; i++)
    {
        new_queue_item->s.unpacked.heights[i] = (typeof(new_queue_item->s.unpacked.heights[i]))solver->initial_lens[i];
    }
    new_queue_item->s.unpacked.foundations = solver->initial_foundation;

    /* Populate the packed item from the unpacked one. */
    memset(&(new_queue_item->s.packed), '\0', sizeof(new_queue_item->s.packed));

    queue_item_populate_packed( solver, new_queue_item );

    memset( &(new_queue_item->rank_counts), '\0',
        sizeof(new_queue_item->rank_counts));

    for (int col_idx = 0 ; col_idx < num_columns ; col_idx++)
    {
        for (int h = 0; h < new_queue_item->s.unpacked.heights[col_idx]; h++)
        {
            new_queue_item->rank_counts.c[
                (ssize_t)solver->board_values[col_idx][h]
            ]++;
        }
    }
    solver->queue_len++;

    return new_queue_item->s.packed;
}

static inline void setup_init_state(
    bhs_solver_t * const solver
)
{
    solver->queue_max_len = 64;
    solver->queue = malloc(sizeof(solver->queue[0]) * (size_t)solver->queue_max_len);
    solver->queue_len = 0;
    solver->num_states_in_collection = 0;

    bhs_state_key_value_pair_t * const init_state = &(solver->init_state);
    *init_state = setup_first_queue_item(solver);

    bh_solve_hash_insert(
        &(solver->positions),
        init_state
    );
    ++solver->num_states_in_collection;
}

static inline void setup_once(
    bhs_solver_t * const solver
)
{
    if (solver->require_initialization)
    {
        setup_init_state(solver);
        solver->require_initialization = FALSE;
    }
}

extern int DLLEXPORT black_hole_solver_run(
    black_hole_solver_instance_t * ret_instance
)
{
    bhs_solver_t * const solver = (bhs_solver_t *)ret_instance;

    setup_once(solver);

    const_SLOT(num_columns, solver);
    const_SLOT(iters_display_step, solver);
    const_SLOT(is_rank_reachability_prune_enabled, solver);
    const_AUTO(max_iters_limit, maxify(solver->max_iters_limit));
    var_AUTO(iterations_num, solver->iterations_num);

    long next_iterations_display_point =
    (
        (iters_display_step <= 0) ? LONG_MAX :
        (iterations_num + iters_display_step - (iterations_num % iters_display_step))
    );

    while (solver->queue_len > 0)
    {
        --solver->queue_len;
        const_AUTO(queue_item_copy, solver->queue[solver->queue_len]);
        const_AUTO(state, queue_item_copy.s.unpacked);
        const_AUTO(foundations, state.foundations);

        if (is_rank_reachability_prune_enabled && (bhs_find_rank_reachability__inline(
                    foundations,
                    queue_item_copy.rank_counts.c
        ) != RANK_REACH__SUCCESS))
        {
            continue;
        }

        iterations_num++;

        fcs_bool_t no_cards = TRUE;

        for (int col_idx = 0 ; col_idx < num_columns ; col_idx++)
        {
            const_AUTO(pos, state.heights[col_idx]);
            if ( pos )
            {
                no_cards = FALSE;
                const_AUTO(card, solver->board_values[col_idx][pos-1]);

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
            solver->final_state = queue_item_copy.s.packed;

            solver->iterations_num = iterations_num;

            return BLACK_HOLE_SOLVER__SUCCESS;
        }
        else if (iterations_num == max_iters_limit)
        {
            solver->iterations_num = iterations_num;

            return BLACK_HOLE_SOLVER__OUT_OF_ITERS;
        }

        if (iterations_num == next_iterations_display_point)
        {
            printf("Iteration: %ld\n", iterations_num);
            fflush(stdout);
            next_iterations_display_point += iters_display_step;
        }
    }

    solver->iterations_num = iterations_num;

    return BLACK_HOLE_SOLVER__NOT_SOLVABLE;
}

extern int DLLEXPORT black_hole_solver_free(
    black_hole_solver_instance_t * instance_proto
)
{
    bhs_solver_t * const solver = (bhs_solver_t *)instance_proto;

    bh_solve_hash_free( &(solver->positions) );

    if (solver->states_in_solution)
    {
        free (solver->states_in_solution);
        solver->states_in_solution = NULL;
    }

    free (solver->queue);
    solver->queue = NULL;

    free (solver);

    return BLACK_HOLE_SOLVER__SUCCESS;
}

#define NUM_STATES_INCREMENT 16

static void initialize_states_in_solution(bhs_solver_t * solver)
{
    if (solver->states_in_solution)
    {
        return;
    }
    int num_states = 0;
    int max_num_states = NUM_SUITS * NUM_RANKS + 1;

    bhs_solution_state_t * states = malloc(sizeof(states[0]) * (size_t)max_num_states);

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
                    sizeof(states[0]) * (size_t)(max_num_states += NUM_STATES_INCREMENT)
                );
        }

        /* Look up the next state in the positions associative array. */
        bh_solve_hash_get(
            &(solver->positions),
            (
                (bhs_state_key_value_pair_t *)
                &(states[num_states].packed.value.parent_state)
            ),
            &(states[num_states+1].packed)
        );
        queue_item_unpack(solver, &states[++num_states]);
    }

    num_states++;

    /* Reverse the list in place. */
    for (int i = 0 ; i < (num_states >> 1) ; i++)
    {
        const_AUTO(temp_state, states[i]);
        states[i] = states[num_states-1-i];
        states[num_states-1-i] = temp_state;
    }

    solver->states_in_solution = states;
    solver->num_states_in_solution = num_states;
    solver->current_state_in_solution_idx = 0;
}

DLLEXPORT extern int black_hole_solver_get_next_move(
    black_hole_solver_instance_t * const instance_proto,
    int * const col_idx_ptr,
    int * const card_rank_ptr,
    int * const card_suit_ptr /*  H=0, C=1, D=2, S=3 */
)
{
    bhs_solver_t * const solver = (bhs_solver_t *)instance_proto;

    initialize_states_in_solution(solver);

    if (solver->current_state_in_solution_idx == solver->num_states_in_solution-1)
    {
        *col_idx_ptr = *card_rank_ptr = *card_suit_ptr = -1;
        return BLACK_HOLE_SOLVER__END;
    }

    {
        const bhs_solution_state_t next_state = solver->states_in_solution[
            ++solver->current_state_in_solution_idx
            ];

        const int col_idx = next_state.packed.value.col_idx;
        const int height = next_state.unpacked.heights[col_idx];

        *col_idx_ptr = col_idx;
        solver->sol_foundations_card_rank = *card_rank_ptr
            = solver->board_values[col_idx][height]+1;
        solver->sol_foundations_card_suit = *card_suit_ptr
            = suit_char_to_index(
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

DLLEXPORT extern int black_hole_solver_get_current_solution_board(
    black_hole_solver_instance_t * instance_proto,
    char * * ptr_to_ret
)
{
    bhs_solver_t * const solver = (bhs_solver_t *)instance_proto;

    initialize_states_in_solution(solver);

    *ptr_to_ret = NULL;

    char * const ret = malloc(
        /* 3 bytes per card. */
        (3 * NUM_SUITS * NUM_RANKS)
            +
        /* newline and a leading ":" per column */
        (2 * BHS__MAX_NUM_COLUMNS)
            +
        /* For the foundations. */
        20
    );

    if (ret == NULL)
    {
        return BLACK_HOLE_SOLVER__OUT_OF_MEMORY;
    }

    char * s = ret;

    s += sprintf(s, "Foundations: ");

    if (solver->sol_foundations_card_rank < 0)
    {
        s += sprintf(s, "-");
    }
    else
    {
        s += sprintf(s, "%c%c",
            (("0A23456789TJQK")[solver->sol_foundations_card_rank]), ("HCDS")[solver->sol_foundations_card_suit]
        );
    }

    s += sprintf(s, "\n");

    bhs_solution_state_t next_state = solver->states_in_solution[
        solver->current_state_in_solution_idx
    ];

    const_SLOT(num_columns, solver);
    for (int col_idx = 0; col_idx < num_columns; col_idx++)
    {
        s += sprintf(s, "%c", ':');
        for (int h = 0; h < next_state.unpacked.heights[col_idx]; h++)
        {
            s += sprintf(s, " %s",
                solver->initial_board_card_strings[col_idx][h]
            );
        }
        s += sprintf(s, "\n");
    }

    *ptr_to_ret = ret;

    return BLACK_HOLE_SOLVER__SUCCESS;
}

DLLEXPORT const char * black_hole_solver_get_lib_version(void)
{
    return VERSION;
}

