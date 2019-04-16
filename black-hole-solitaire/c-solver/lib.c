// This file is part of Black Hole Solitaire Solver. It is subject to the
// license terms in the COPYING file found in the top-level directory of this
// distribution and at
// https://www.shlomifish.org/open-source/projects/black-hole-solitaire-solver/
// . No part of Black Hole Solitaire Solver, including this file, may be
// copied, modified, propagated, or distributed except according to the terms
// contained in the COPYING file.
//
// Copyright (c) 2010 Shlomi Fish

// black_hole_solver.c - a solver for Black Hole Solitaire.

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <ctype.h>
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>

#include "config.h"
#include <black-hole-solver/black_hole_solver.h>
#include "can_move.h"
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
    uint8_t heights[BHS__MAX_NUM_COLUMNS];
    int8_t foundations;
    uint8_t talon_ptr;
} bhs_unpacked_state_t;

typedef struct
{
    bhs_state_key_value_pair_t packed;
    bhs_unpacked_state_t unpacked;
} bhs_solution_state_t;

typedef struct
{
    bhs_solution_state_t s;
    bhs_rank_counts rank_counts;
} bhs_queue_item_t;

#define TALON_MAX_SIZE (NUM_RANKS * 4)
#define QUEUE_MAX_SIZE (1 + (BHS__MAX_NUM_COLUMNS + 1) * NUM_RANKS * NUM_SUITS)

typedef const bool can_move__row[13];
typedef struct
{
    uint_fast16_t talon_len;
    bh_solve_hash_t positions;
    meta_allocator meta_alloc;
    uint_fast16_t initial_lens[BHS__MAX_NUM_COLUMNS];
    uint_fast32_t num_states_in_solution, current_state_in_solution_idx;
    unsigned long iterations_num, num_states_in_collection, max_iters_limit;
    uint_fast32_t num_columns;
    uint_fast32_t bits_per_column;
    uint_fast32_t queue_len;
    int_fast32_t sol_foundations_card_rank, sol_foundations_card_suit;
    // This is the ranks of the cards in the columns. It remains constant
    // for the duration of the game.
    bhs_rank_t board_ranks[BHS__MAX_NUM_COLUMNS][BHS__MAX_NUM_CARDS_IN_COL];

    bhs_rank_t initial_foundation;
    bhs_rank_t talon_values[TALON_MAX_SIZE];

    bhs_card_string_t initial_foundation_string;
    bhs_card_string_t initial_board_card_strings[BHS__MAX_NUM_COLUMNS]
                                                [BHS__MAX_NUM_CARDS_IN_COL];
    bhs_card_string_t initial_talon_card_strings[TALON_MAX_SIZE];

    bhs_state_key_value_pair_t init_state;
    bhs_state_key_value_pair_t final_state;

    bool is_rank_reachability_prune_enabled;
    bool effective_is_rank_reachability_prune_enabled;
    bool place_queens_on_kings;
    bool wrap_ranks;
    bool effective_place_queens_on_kings;
    can_move__row *can_move;
    bhs_queue_item_t queue[QUEUE_MAX_SIZE];
#define max_num_states (NUM_SUITS * NUM_RANKS + 1)
    bhs_solution_state_t states_in_solution[max_num_states];
} bhs_solver_t;

int DLLEXPORT black_hole_solver_create(
    black_hole_solver_instance_t **ret_instance)
{
    bhs_solver_t *const ret = (bhs_solver_t *)malloc(sizeof(*ret));

    if (!ret)
    {
        *ret_instance = NULL;
        return BLACK_HOLE_SOLVER__OUT_OF_MEMORY;
    }
    ret->iterations_num = 0;
    ret->num_states_in_collection = 0;
    ret->max_iters_limit = ULONG_MAX;
    ret->is_rank_reachability_prune_enabled = false;
    ret->num_columns = 0;
    ret->place_queens_on_kings = false;
    ret->wrap_ranks = true;

    fc_solve_meta_compact_allocator_init(&(ret->meta_alloc));
    bh_solve_hash_init(&(ret->positions), &(ret->meta_alloc));

    *ret_instance = (black_hole_solver_instance_t *)ret;

    return BLACK_HOLE_SOLVER__SUCCESS;
}

DLLEXPORT extern int black_hole_solver_enable_place_queens_on_kings(
    black_hole_solver_instance_t *const instance_proto,
    const bool enabled_status)
{
    bhs_solver_t *const solver = (bhs_solver_t *)instance_proto;
    solver->place_queens_on_kings = enabled_status ? true : false;

    return BLACK_HOLE_SOLVER__SUCCESS;
}

DLLEXPORT extern int black_hole_solver_enable_wrap_ranks(
    black_hole_solver_instance_t *const instance_proto,
    const bool enabled_status)
{
    bhs_solver_t *const solver = (bhs_solver_t *)instance_proto;
    solver->wrap_ranks = enabled_status ? true : false;

    return BLACK_HOLE_SOLVER__SUCCESS;
}

DLLEXPORT extern int black_hole_solver_enable_rank_reachability_prune(
    black_hole_solver_instance_t *const instance_proto,
    const bool enabled_status)
{
    bhs_solver_t *const solver = (bhs_solver_t *)instance_proto;
    solver->is_rank_reachability_prune_enabled = enabled_status ? true : false;

    return BLACK_HOLE_SOLVER__SUCCESS;
}

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

static int parse_card(const char **s, bhs_rank_t *const foundation,
    bhs_card_string_t card, int_fast32_t *const suit_ptr)
{
    strncpy(card, (*s), BHS_CARD_STRING_LEN);
    card[BHS_CARD_STRING_LEN] = '\0';

    /* Short for value. */
    bhs_rank_t v;

    switch (*(*s))
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

    ++(*s);

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
    ++(*s);

    return BLACK_HOLE_SOLVER__SUCCESS;
}

static inline bool string_find_prefix(const char **s, const char *const prefix)
{
    register size_t len = strlen(prefix);

    if (strncmp(*s, prefix, len) != 0)
    {
        return false;
    }

    (*s) += len;

    return true;
}

#define MYRET(code)                                                            \
    {                                                                          \
        *error_line_number = line_num;                                         \
        return code;                                                           \
    }

static int try_parse_talon(bhs_solver_t *const solver, const char **ps)
{
    if (string_find_prefix(ps, "Talon: "))
    {
        size_t pos_idx = 0;
        while ((**ps != '\n') && (**ps != '\0'))
        {
            if (pos_idx == TALON_MAX_SIZE)
            {
                return BLACK_HOLE_SOLVER__TOO_MANY_CARDS;
            }

            const int ret_code =
                parse_card(ps, &(solver->talon_values[pos_idx]),
                    solver->initial_talon_card_strings[pos_idx], NULL);

            if (ret_code)
            {
                return ret_code;
            }

            while ((**ps) == ' ')
            {
                ++(*ps);
            }

            ++pos_idx;
        }
        if (**ps == '\n')
        {
            ++(*ps);
        }

        solver->talon_len = pos_idx;
    }
    return BLACK_HOLE_SOLVER__SUCCESS;
}

extern int DLLEXPORT black_hole_solver_read_board(
    black_hole_solver_instance_t *const instance_proto,
    const char *const board_string, int *const error_line_number,
    const unsigned int num_columns, const unsigned int max_num_cards_in_col,
    const unsigned int bits_per_column)
{
    int line_num = 1;

    if (num_columns > BHS__MAX_NUM_COLUMNS)
    {
        return BLACK_HOLE_SOLVER__INVALID_INPUT;
    }

    if (max_num_cards_in_col > BHS__MAX_NUM_CARDS_IN_COL)
    {
        return BLACK_HOLE_SOLVER__INVALID_INPUT;
    }

    bhs_solver_t *const solver = (bhs_solver_t *)instance_proto;

    solver->num_columns = num_columns;
    solver->bits_per_column = bits_per_column;

    const char *s = board_string;

    /* Read the foundations. */

    while ((*s) == '\n')
    {
        ++line_num;
        ++s;
    }

    solver->talon_len = 0;
    {
        const int ret_code = try_parse_talon(solver, &s);
        if (ret_code)
        {
            MYRET(ret_code);
        }
    }
    if (!string_find_prefix(&s, "Foundations: "))
    {
        MYRET(BLACK_HOLE_SOLVER__FOUNDATIONS_NOT_FOUND_AT_START);
    }

    while (isspace(*s) && ((*s) != '\n'))
    {
        ++s;
    }

    if ((*s) == '-')
    {
        /* A non-initialized foundation. */
        solver->initial_foundation_string[0] = '\0';
        solver->initial_foundation = -1;
        solver->sol_foundations_card_rank = -1;
        solver->sol_foundations_card_suit = -1;
        ++s;
    }
    else
    {
        const int ret_code = parse_card(&s, &(solver->initial_foundation),
            solver->initial_foundation_string,
            &(solver->sol_foundations_card_suit));

        solver->sol_foundations_card_rank = 1 + solver->initial_foundation;

        if (ret_code)
        {
            MYRET(ret_code);
        }
    }

    if (*(s++) != '\n')
    {
        MYRET(BLACK_HOLE_SOLVER__TRAILING_CHARS);
    }
    ++line_num;
    {
        const int ret_code = try_parse_talon(solver, &s);
        if (ret_code)
        {
            MYRET(ret_code);
        }
    }

    for (size_t col_idx = 0; col_idx < num_columns; ++col_idx, ++line_num)
    {
        unsigned int pos_idx = 0;
        while ((*s != '\n') && (*s != '\0'))
        {
            if (pos_idx == max_num_cards_in_col)
            {
                MYRET(BLACK_HOLE_SOLVER__TOO_MANY_CARDS);
            }

            const int ret_code =
                parse_card(&s, &(solver->board_ranks[col_idx][pos_idx]),
                    solver->initial_board_card_strings[col_idx][pos_idx], NULL);

            if (ret_code)
            {
                MYRET(ret_code);
            }

            while ((*s) == ' ')
            {
                ++s;
            }

            ++pos_idx;
        }

        solver->initial_lens[col_idx] = pos_idx;

        if (*s == '\0')
        {
            MYRET(BLACK_HOLE_SOLVER__NOT_ENOUGH_COLUMNS);
        }
        else
        {
            ++s;
        }
    }

    *error_line_number = -1;
    return BLACK_HOLE_SOLVER__SUCCESS;
}

#undef MYRET

DLLEXPORT extern int black_hole_solver_set_max_iters_limit(
    black_hole_solver_instance_t *const instance_proto,
    const unsigned long limit)
{
    ((bhs_solver_t *const)instance_proto)->max_iters_limit = limit;

    return BLACK_HOLE_SOLVER__SUCCESS;
}

#define TALON_PTR_BITS 6

static inline void queue_item_populate_packed(
    bhs_solver_t *const solver, bhs_queue_item_t *const queue_item)
{
    queue_item->s.packed.key.foundations = queue_item->s.unpacked.foundations;

    fc_solve_bit_writer_t bit_w;
    fc_solve_bit_writer_init(&bit_w, queue_item->s.packed.key.data);

    const_SLOT(num_columns, solver);
    const_SLOT(bits_per_column, solver);
    fc_solve_bit_writer_write(
        &bit_w, TALON_PTR_BITS, queue_item->s.unpacked.talon_ptr);
    for (size_t col = 0; col < num_columns; ++col)
    {
        fc_solve_bit_writer_write(
            &bit_w, bits_per_column, queue_item->s.unpacked.heights[col]);
    }
}

static inline void queue_item_unpack(
    bhs_solver_t *const solver, bhs_solution_state_t *const queue_item)
{
    const_SLOT(num_columns, solver);
    const_SLOT(bits_per_column, solver);

    queue_item->unpacked.foundations = queue_item->packed.key.foundations;

    fc_solve_bit_reader_t bit_r;
    fc_solve_bit_reader_init(&bit_r, queue_item->packed.key.data);

    queue_item->unpacked.talon_ptr =
        (uint8_t)fc_solve_bit_reader_read(&bit_r, TALON_PTR_BITS);
    for (size_t col = 0; col < num_columns; ++col)
    {
        queue_item->unpacked.heights[col] =
            (typeof(queue_item->unpacked.heights[col]))fc_solve_bit_reader_read(
                &bit_r, bits_per_column);
    }
}

static inline void perform_move(bhs_solver_t *const solver,
    const bhs_rank_t prev_foundation, const bhs_rank_t card,
    const uint_fast32_t col_idx,
    const bhs_queue_item_t *const queue_item_copy_ptr)
{
    const_SLOT(num_columns, solver);
    bhs_unpacked_state_t next_state = queue_item_copy_ptr->s.unpacked;

    next_state.foundations = card;
    if (col_idx == num_columns)
    {
        ++next_state.talon_ptr;
    }
    else
    {
        --next_state.heights[col_idx];
    }

    bhs_queue_item_t next_queue_item;

    next_queue_item.s.unpacked = next_state;
    memset(&(next_queue_item.s.packed), '\0', sizeof(next_queue_item.s.packed));

    next_queue_item.s.packed.value.col_idx =
        (typeof(next_queue_item.s.packed.value.col_idx))col_idx;
    next_queue_item.s.packed.value.prev_foundation = prev_foundation;

    queue_item_populate_packed(solver, &(next_queue_item));

    next_queue_item.rank_counts = queue_item_copy_ptr->rank_counts;
    next_queue_item.rank_counts.c[(ssize_t)card]--;

    if (!bh_solve_hash_insert(
            &(solver->positions), &(next_queue_item.s.packed)))
    {
        ++solver->num_states_in_collection;
        /* It's a new state - put it in the queue. */
        solver->queue[(solver->queue_len)++] = next_queue_item;

        assert(solver->queue_len < QUEUE_MAX_SIZE);
    }
}

static inline bhs_state_key_value_pair_t setup_first_queue_item(
    bhs_solver_t *const solver)
{
    const_SLOT(num_columns, solver);

    typeof(solver->queue[solver->queue_len]) *const new_queue_item =
        &(solver->queue[solver->queue_len]);

    // Populate the unpacked state.
    for (size_t i = 0; i < num_columns; ++i)
    {
        new_queue_item->s.unpacked.heights[i] = (typeof(
            new_queue_item->s.unpacked.heights[i]))solver->initial_lens[i];
    }
    new_queue_item->s.unpacked.foundations = solver->initial_foundation;
    new_queue_item->s.unpacked.talon_ptr = 0;

    // Populate the packed item from the unpacked one.
    memset(&(new_queue_item->s.packed), '\0', sizeof(new_queue_item->s.packed));

    queue_item_populate_packed(solver, new_queue_item);
    new_queue_item->s.packed.value.col_idx = (bhs_col_idx_t)num_columns + 1;

    memset(&(new_queue_item->rank_counts), '\0',
        sizeof(new_queue_item->rank_counts));

    for (size_t col_idx = 0; col_idx < num_columns; ++col_idx)
    {
        for (size_t h = 0; h < new_queue_item->s.unpacked.heights[col_idx]; ++h)
        {
            ++(new_queue_item->rank_counts
                    .c[(ssize_t)solver->board_ranks[col_idx][h]]);
        }
    }
    ++solver->queue_len;

    return new_queue_item->s.packed;
}

static inline void setup_config(bhs_solver_t *const solver)
{
    solver->queue_len = 0;
    solver->num_states_in_collection = 0;
    solver->effective_place_queens_on_kings =
        (solver->place_queens_on_kings || solver->wrap_ranks);
    solver->can_move = &(black_hole_solver__can_move[solver->wrap_ranks][1]);
}

static inline void setup_init_state(bhs_solver_t *const solver)
{
    bhs_state_key_value_pair_t *const init_state = &(solver->init_state);
    *init_state = setup_first_queue_item(solver);

    bh_solve_hash_insert(&(solver->positions), init_state);
    ++solver->num_states_in_collection;
    solver->effective_is_rank_reachability_prune_enabled =
        solver->talon_len ? false : solver->is_rank_reachability_prune_enabled;
}

extern int DLLEXPORT black_hole_solver_config_setup(
    black_hole_solver_instance_t *instance_proto)
{
    bhs_solver_t *const solver = (bhs_solver_t *)instance_proto;
    setup_config(solver);
    return BLACK_HOLE_SOLVER__SUCCESS;
}

extern int DLLEXPORT black_hole_solver_setup(
    black_hole_solver_instance_t *instance_proto)
{
    bhs_solver_t *const solver = (bhs_solver_t *)instance_proto;
    setup_init_state(solver);
    return BLACK_HOLE_SOLVER__SUCCESS;
}

extern int DLLEXPORT black_hole_solver_run(
    black_hole_solver_instance_t *instance_proto)
{
    bhs_solver_t *const solver = (bhs_solver_t *)instance_proto;

    const_SLOT(can_move, solver);

    const_SLOT(num_columns, solver);
    const_SLOT(talon_len, solver);
    const_SLOT(effective_is_rank_reachability_prune_enabled, solver);
    const_SLOT(effective_place_queens_on_kings, solver);
    const_SLOT(max_iters_limit, solver);
    var_AUTO(iterations_num, solver->iterations_num);

    while (solver->queue_len > 0)
    {
        --solver->queue_len;
        const_AUTO(queue_item_copy, solver->queue[solver->queue_len]);
        const_AUTO(state, queue_item_copy.s.unpacked);
        const_AUTO(foundations, state.foundations);
        const_AUTO(talon_ptr, state.talon_ptr);

        if (effective_is_rank_reachability_prune_enabled &&
            (bhs_find_rank_reachability__inline(foundations,
                 &queue_item_copy.rank_counts) != RANK_REACH__SUCCESS))
        {
            continue;
        }

        ++iterations_num;

        bool no_cards = true;
        const bool has_talon = talon_ptr < talon_len;

        if (has_talon)
        {
            perform_move(solver, foundations, solver->talon_values[talon_ptr],
                num_columns, &queue_item_copy);
        }
        if (effective_place_queens_on_kings || (foundations != RANK_K))
        {
            const bool *const found_can_move = can_move[foundations];
            for (uint_fast32_t col_idx = 0; col_idx < num_columns; ++col_idx)
            {
                const_AUTO(pos, state.heights[col_idx]);
                if (pos)
                {
                    no_cards = false;
                    const_AUTO(card, solver->board_ranks[col_idx][pos - 1]);

                    if (found_can_move[(size_t)card])
                    {
                        perform_move(solver, foundations, card, col_idx,
                            &queue_item_copy);
                    }
                }
            }
        }
        else
        {
            for (size_t col_idx = 0; col_idx < num_columns; ++col_idx)
            {
                const_AUTO(pos, state.heights[col_idx]);
                if (pos)
                {
                    no_cards = false;
                    break;
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
    }

    solver->iterations_num = iterations_num;

    return BLACK_HOLE_SOLVER__NOT_SOLVABLE;
}

extern int DLLEXPORT black_hole_solver_recycle(
    black_hole_solver_instance_t *instance_proto)
{
    bhs_solver_t *const solver = (bhs_solver_t *)instance_proto;

    bh_solve_hash_recycle(&(solver->positions));
    solver->iterations_num = 0;
    solver->queue_len = 0;
    solver->num_states_in_collection = 0;

    return BLACK_HOLE_SOLVER__SUCCESS;
}

#define NUM_STATES_INCREMENT 16

DLLEXPORT void black_hole_solver_init_solution_moves(
    black_hole_solver_instance_t *instance_proto)
{
    bhs_solver_t *const solver = (bhs_solver_t *)instance_proto;
    const_SLOT(num_columns, solver);
    const_SLOT(bits_per_column, solver);
    uint_fast32_t num_states = 0;

    bhs_solution_state_t *const states = solver->states_in_solution;

    states[num_states].packed = (solver->final_state);
    queue_item_unpack(solver, &states[num_states]);

    while (memcmp(&(states[num_states].packed.key), &(solver->init_state.key),
        sizeof(states[num_states].packed.key)))
    {
        assert(num_states < max_num_states);
        /* Look up the next state in the positions associative array. */
        bh_solve_hash_get(&(solver->positions),
            &(states[num_states].packed.key),
            &(states[num_states + 1].packed.value));
        // Reverse the move
        const size_t col_idx = states[num_states + 1].packed.value.col_idx;
        states[num_states + 1].packed.key = states[num_states].packed.key;
        if (col_idx == num_columns + 1)
        {
            states[num_states + 1].packed.key.foundations =
                solver->initial_foundation;
        }
        else if (col_idx == num_columns)
        {
            const_AUTO(
                moved_card_height, states[num_states].unpacked.talon_ptr);
            var_AUTO(new_moved_card_height, moved_card_height - 1);
            states[num_states + 1].packed.key.foundations =
                states[num_states + 1].packed.value.prev_foundation;
            unsigned char *data = states[num_states + 1].packed.key.data;
            for (size_t i = 0; i < TALON_PTR_BITS;
                 ++i, new_moved_card_height >>= 1)
            {
                data[i >> 3] &= (~(1 << (i & 0x7)));
                data[i >> 3] |= ((new_moved_card_height & 0x1) << (i & 0x7));
            }
        }
        else
        {
            const_AUTO(moved_card_height,
                states[num_states].unpacked.heights[col_idx]);
            var_AUTO(new_moved_card_height, moved_card_height + 1);
            states[num_states + 1].packed.key.foundations =
                states[num_states + 1].packed.value.prev_foundation;
            var_AUTO(offset, TALON_PTR_BITS + bits_per_column * col_idx);
            unsigned char *data = states[num_states + 1].packed.key.data;
            for (size_t i = 0; i < bits_per_column;
                 ++i, ++offset, new_moved_card_height >>= 1)
            {
                data[offset >> 3] &= (~(1 << (offset & 0x7)));
                data[offset >> 3] |=
                    ((new_moved_card_height & 0x1) << (offset & 0x7));
            }
        }

        queue_item_unpack(solver, &states[++num_states]);
    }
    states[num_states].packed.key.foundations =
        states[num_states].unpacked.foundations = solver->initial_foundation;

    ++num_states;
    const_AUTO(lim, (num_states >> 1));
    // Reverse the list in place.
    for (size_t i = 0; i < lim; ++i)
    {
        const_AUTO(temp_state, states[i]);
        states[i] = states[num_states - 1 - i];
        states[num_states - 1 - i] = temp_state;
    }

    solver->num_states_in_solution = num_states;
    solver->current_state_in_solution_idx = 0;
}

DLLEXPORT extern int black_hole_solver_get_next_move(
    black_hole_solver_instance_t *const instance_proto, int *const col_idx_ptr,
    int *const card_rank_ptr, int *const card_suit_ptr /*  H=0, C=1, D=2, S=3 */
)
{
    bhs_solver_t *const solver = (bhs_solver_t *)instance_proto;

    if (solver->current_state_in_solution_idx ==
        solver->num_states_in_solution - 1)
    {
        *col_idx_ptr = *card_rank_ptr = *card_suit_ptr = -1;
        return BLACK_HOLE_SOLVER__END;
    }

    {
        const bhs_solution_state_t next_state =
            solver->states_in_solution[solver->current_state_in_solution_idx++];

        const uint_fast32_t col_idx = next_state.packed.value.col_idx;
        const bool is_talon = (col_idx == solver->num_columns);
        const uint_fast16_t height =
            (is_talon ? (next_state.unpacked.talon_ptr)
                      : (next_state.unpacked.heights[col_idx] - 1));
        assert(height <
               (is_talon ? solver->talon_len : solver->initial_lens[col_idx]));

        *col_idx_ptr = (int)col_idx;
        solver->sol_foundations_card_rank = *card_rank_ptr =
            (is_talon ? solver->talon_values
                      : solver->board_ranks[col_idx])[height] +
            1;
        solver->sol_foundations_card_suit = *card_suit_ptr = suit_char_to_index(
            (is_talon
                    ? solver->initial_talon_card_strings
                    : solver->initial_board_card_strings[col_idx])[height][1]);

        return BLACK_HOLE_SOLVER__SUCCESS;
    }
}

DLLEXPORT extern unsigned long __attribute__((pure))
black_hole_solver_get_num_states_in_collection(
    black_hole_solver_instance_t *const instance_proto)
{
    return ((bhs_solver_t *)instance_proto)->num_states_in_collection;
}

DLLEXPORT extern unsigned long __attribute__((pure))
black_hole_solver_get_iterations_num(
    black_hole_solver_instance_t *instance_proto)
{
    return ((bhs_solver_t *)instance_proto)->iterations_num;
}

DLLEXPORT extern int black_hole_solver_get_current_solution_board(
    black_hole_solver_instance_t *instance_proto, char *const output)
{
    bhs_solver_t *const solver = (bhs_solver_t *)instance_proto;

    char *s = output;

    s += sprintf(s, "Foundations: ");

    if (solver->sol_foundations_card_rank < 0)
    {
        s += sprintf(s, "-");
    }
    else
    {
        s += sprintf(s, "%c%c",
            (("0A23456789TJQK")[solver->sol_foundations_card_rank]),
            ("HCDS")[solver->sol_foundations_card_suit]);
    }

    s += sprintf(s, "\n");

    bhs_solution_state_t next_state =
        solver->states_in_solution[solver->current_state_in_solution_idx];

    const_SLOT(talon_len, solver);
    if (talon_len)
    {
        s += sprintf(s, "%s", "Talon:");
        for (uint_fast16_t h = next_state.unpacked.talon_ptr; h < talon_len;
             ++h)
        {
            s += sprintf(s, " %s", solver->initial_talon_card_strings[h]);
        }
        s += sprintf(s, "\n");
    }
    const_SLOT(num_columns, solver);
    for (size_t col_idx = 0; col_idx < num_columns; ++col_idx)
    {
        s += sprintf(s, "%c", ':');
        for (size_t h = 0; h < next_state.unpacked.heights[col_idx]; ++h)
        {
            s += sprintf(
                s, " %s", solver->initial_board_card_strings[col_idx][h]);
        }
        s += sprintf(s, "\n");
    }

    return BLACK_HOLE_SOLVER__SUCCESS;
}

DLLEXPORT const __attribute__((const)) char *black_hole_solver_get_lib_version(
    void)
{
    return VERSION;
}

extern int DLLEXPORT black_hole_solver_free(
    black_hole_solver_instance_t *instance_proto)
{
    bhs_solver_t *const solver = (bhs_solver_t *)instance_proto;

    bh_solve_hash_free(&(solver->positions));
    fc_solve_meta_compact_allocator_finish(&(solver->meta_alloc));

    free(solver);

    return BLACK_HOLE_SOLVER__SUCCESS;
}
