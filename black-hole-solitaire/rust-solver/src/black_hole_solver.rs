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
// black_hole_solver.c - a solver for Black Hole Solitaire.

use std::collections::HashMap;

const NUM_SUITS:usize = 4;
enum BHS_SUITS
{
    SUIT_H,
    SUIT_C,
    SUIT_D,
    SUIT_S,
    INVALID_SUIT = -1,
}

fn suit_char_to_index(suit:char)->BHS_SUITS
{
    return match suit
    {
        'H'=> SUIT_H,
    'C' =>SUIT_C,
    'D' =>SUIT_D,
    'S' =>SUIT_S,
    _=> INVALID_SUIT,
    };
}

struct  bhs_unpacked_state_t
{
    heights:[u8;BHS__MAX_NUM_COLUMNS],
    foundations:i8,
}

struct bhs_solution_state_t
{
    packed: bhs_state_key_value_pair_t,

    unpacked: bhs_unpacked_state_t,
}

struct  bhs_rank_counts_t
{
    c:[u8;NUM_RANKS],
}

struct  bhs_queue_item_t
{
    s:bhs_solution_state_t,
     rank_counts:bhs_rank_counts_t,
}

struct bhs_solver_t
{
    /*
     * TODO : rename from board_values.
     *
     * This is the ranks of the cards in the columns. It remains constant
     * for the duration of the game.
     * */
     board_values:[[bhs_rank_t;BHS__MAX_NUM_COLUMNS];BHS__MAX_NUM_CARDS_IN_COL],

    initial_foundation:bhs_rank_t,

    positions: HashMap<bhs_state_key_t, bhs_state_value_t>,

    initial_foundation_string: bhs_card_string_t,
   initial_board_card_strings:[[bhs_card_string_t;BHS__MAX_NUM_COLUMNS];BHS__MAX_NUM_CARDS_IN_COL],
    initial_lens:[usize;BHS__MAX_NUM_COLUMNS],

    init_state: bhs_state_key_value_pair_t,
    final_state: bhs_state_key_value_pair_t,

    states_in_solution: [bhs_solution_state_t;64],
    num_states_in_solution: usize,
    current_state_in_solution_idx: usize,

    iterations_num: usize,
    num_states_in_collection: usize,
    max_iters_limit: usize,
    iters_display_step: usize,

    num_columns: usize,
    bits_per_column: usize,

    queue: [bhs_queue_item_t;53],
    queue_len: usize,
    queue_max_len: usize,
    sol_foundations_card_rank: usize,
    sol_foundations_card_suit: usize,
    is_rank_reachability_prune_enabled: bool,
    require_initialization: bool,
}

impl bhs_solver_t {
    fn black_hole_solver_create()->bhs_solver_t
{
    bhs_solver_t {
    require_initialization:TRUE,
    states_in_solution:NULL,
    iterations_num:0,
    num_states_in_collection:0,
    max_iters_limit:-1,
    is_rank_reachability_prune_enabled:FALSE,
    iters_display_step:0,
    num_columns:0,
    queue:NULL,
}
}

 fn black_hole_solver_enable_rank_reachability_prune(&self,
    enabled_status:bool) -> usize
{
    self.is_rank_reachability_prune_enabled = enabled_status;

    return BLACK_HOLE_SOLVER__SUCCESS;
}

const MAX_RANK:usize = (NUM_RANKS - 1);

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
}

enum BHS_PARSE_VERDICT
{
    BLACK_HOLE_SOLVER__SUCCESS,
    BLACK_HOLE_SOLVER__UNKNOWN_SUIT,
    BLACK_HOLE_SOLVER__UNKNOWN_RANK,
}

fn parse_card(s: String, card:bhs_card_string_t) -> (BHS_PARSE_VERDICT, bhs_rank_t, BHS_SUITS)
{
    /* Short for value. */
    let mut v: bhs_rank_t;

    match (s[0])
    {
    'A'=>
        v = RANK_A,

    '2'=>
        v = RANK_2,

    '3'=>
        v = RANK_3,

    '4'=>
        v = RANK_4,

    '5'=>
        v = RANK_5,

    '6'=>
        v = RANK_6,

    '7'=>
        v = RANK_7,

    '8'=>
        v = RANK_8,

    '9'=>
        v = RANK_9,

    'T'=>
        v = RANK_T,

    'J'=>
        v = RANK_J,

    'Q'=>
        v = RANK_Q,

    'K'=>
        v = RANK_K,

    _=>
        return (BLACK_HOLE_SOLVER__UNKNOWN_RANK, RANK_A, INVALID_SUIT),
    };

    let mut suit_ptr: BHS_SUITS;
    match (s[1])
    {
    'H'|'S'|'D'|'C'=>
            suit_ptr = suit_char_to_index(s[1]),
            _=>
        return (BLACK_HOLE_SOLVER__UNKNOWN_SUIT, RANK_A, INVALID_SUIT),
    }

    return (BLACK_HOLE_SOLVER__SUCCESS, v, suit_ptr);
}

fn string_find_prefix(s: String, start: usize, prefix: String) -> (bool, usize)
{
    let len: usize = strlen(prefix);

    if (s[start..start+len] != prefix)
    {
        return (FALSE, start);
    }
    return (TRUE, start+len);
}

fn black_hole_solver_read_board(
    black_hole_solver_instance_t *const instance_proto,
    const char *const board_string, int *const error_line_number,
    const int num_columns, const int max_num_cards_in_col,
    const int bits_per_column)
{
    int line_num = 1;

    if (num_columns > BHS__MAX_NUM_COLUMNS)
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
        line_num++;
        s++;
    }

    if (!string_find_prefix(&s, "Foundations: "))
    {
        error_line_number = line_num;
        return BLACK_HOLE_SOLVER__FOUNDATIONS_NOT_FOUND_AT_START;
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
        const int ret_code = parse_card(&s, &(solver->initial_foundation),
            solver->initial_foundation_string,
            &(solver->sol_foundations_card_suit));

        solver->sol_foundations_card_rank = solver->initial_foundation;

        if (ret_code)
        {
            error_line_number = line_num;
            return ret_code;
        }
    }

    if (*(s++) != '\n')
    {
        error_line_number = line_num;
        return BLACK_HOLE_SOLVER__TRAILING_CHARS;
    }
    line_num++;

    for (int col_idx = 0; col_idx < num_columns; col_idx++, line_num++)
    {
        int pos_idx = 0;
        while ((*s != '\n') && (*s != '\0'))
        {
            if (pos_idx == max_num_cards_in_col)
            {
                error_line_number = line_num;
                return BLACK_HOLE_SOLVER__TOO_MANY_CARDS;
            }

            const int ret_code =
                parse_card(&s, &(solver->board_values[col_idx][pos_idx]),
                    solver->initial_board_card_strings[col_idx][pos_idx], NULL);

            if (ret_code)
            {
                error_line_number = line_num;
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
            error_line_number = line_num;
            return BLACK_HOLE_SOLVER__NOT_ENOUGH_COLUMNS;
        }
        else
        {
            s++;
        }
    }

    *error_line_number = -1;
    return BLACK_HOLE_SOLVER__SUCCESS;
}

fn black_hole_solver_set_max_iters_limit(
    black_hole_solver_instance_t *const instance_proto, const long limit)
{
    ((bhs_solver_t *const)instance_proto)->max_iters_limit = limit;

    return BLACK_HOLE_SOLVER__SUCCESS;
}

fn black_hole_solver_set_iters_display_step(
    black_hole_solver_instance_t *const instance_proto,
    const long iters_display_step)
{
    if (iters_display_step < 0)
    {
        return BLACK_HOLE_SOLVER__INVALID_INPUT;
    }
    ((bhs_solver_t *const)instance_proto)->iters_display_step =
        iters_display_step;

    return BLACK_HOLE_SOLVER__SUCCESS;
}

static inline void queue_item_populate_packed(
    bhs_solver_t *const solver, bhs_queue_item_t *const queue_item)
{
    queue_item->s.packed.key.foundations = queue_item->s.unpacked.foundations;

    fc_solve_bit_writer_t bit_w;
    fc_solve_bit_writer_init(&bit_w, queue_item->s.packed.key.data);

    const_SLOT(num_columns, solver);
    const_SLOT(bits_per_column, solver);
    for (int col = 0; col < num_columns; col++)
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

    for (int col = 0; col < num_columns; col++)
    {
        queue_item->unpacked.heights[col] =
            (typeof(queue_item->unpacked.heights[col]))fc_solve_bit_reader_read(
                &bit_r, bits_per_column);
    }
}

static inline void perform_move(bhs_solver_t *const solver,
    const bhs_rank_t card, const int col_idx,
    const bhs_queue_item_t *const queue_item_copy_ptr)
{
    bhs_unpacked_state_t next_state = queue_item_copy_ptr->s.unpacked;

    next_state.foundations = card;
    next_state.heights[col_idx]--;

    bhs_queue_item_t next_queue_item;

    next_queue_item.s.unpacked = next_state;
    memset(&(next_queue_item.s.packed), '\0', sizeof(next_queue_item.s.packed));

    next_queue_item.s.packed.value.parent_state =
        queue_item_copy_ptr->s.packed.key;
    next_queue_item.s.packed.value.col_idx =
        (typeof(next_queue_item.s.packed.value.col_idx))col_idx;

    queue_item_populate_packed(solver, &(next_queue_item));

    next_queue_item.rank_counts = queue_item_copy_ptr->rank_counts;
    next_queue_item.rank_counts.c[(ssize_t)card]--;

    if (!bh_solve_hash_insert(
            &(solver->positions), &(next_queue_item.s.packed)))
    {
        solver->num_states_in_collection++;
        /* It's a new state - put it in the queue. */
        solver->queue[(solver->queue_len)++] = next_queue_item;

        if (solver->queue_len == solver->queue_max_len)
        {
            solver->queue = realloc(
                solver->queue, sizeof(solver->queue[0]) *
                                   (size_t)(solver->queue_max_len += 64));
        }
    }
}

static inline long maxify(long n) { return ((n < 0) ? LONG_MAX : n); }

static inline bhs_state_key_value_pair_t setup_first_queue_item(
    bhs_solver_t *const solver)
{
    const_SLOT(num_columns, solver);

    typeof(solver->queue[solver->queue_len]) *const new_queue_item =
        &(solver->queue[solver->queue_len]);

    /* Populate the unpacked state. */
    for (int i = 0; i < num_columns; i++)
    {
        new_queue_item->s.unpacked.heights[i] = (typeof(
            new_queue_item->s.unpacked.heights[i]))solver->initial_lens[i];
    }
    new_queue_item->s.unpacked.foundations = solver->initial_foundation;

    /* Populate the packed item from the unpacked one. */
    memset(&(new_queue_item->s.packed), '\0', sizeof(new_queue_item->s.packed));

    queue_item_populate_packed(solver, new_queue_item);

    memset(&(new_queue_item->rank_counts), '\0',
        sizeof(new_queue_item->rank_counts));

    for (int col_idx = 0; col_idx < num_columns; col_idx++)
    {
        for (int h = 0; h < new_queue_item->s.unpacked.heights[col_idx]; h++)
        {
            new_queue_item->rank_counts
                .c[(ssize_t)solver->board_values[col_idx][h]]++;
        }
    }
    solver->queue_len++;

    return new_queue_item->s.packed;
}

static inline void setup_init_state(bhs_solver_t *const solver)
{
    solver->queue_max_len = 64;
    solver->queue =
        malloc(sizeof(solver->queue[0]) * (size_t)solver->queue_max_len);
    solver->queue_len = 0;
    solver->num_states_in_collection = 0;

    bhs_state_key_value_pair_t *const init_state = &(solver->init_state);
    *init_state = setup_first_queue_item(solver);

    bh_solve_hash_insert(&(solver->positions), init_state);
    ++solver->num_states_in_collection;
}

static inline void setup_once(bhs_solver_t *const solver)
{
    if (solver->require_initialization)
    {
        setup_init_state(solver);
        solver->require_initialization = FALSE;
    }
}

fn black_hole_solver_run(
    black_hole_solver_instance_t *ret_instance)
{
    bhs_solver_t *const solver = (bhs_solver_t *)ret_instance;

    setup_once(solver);

    const_SLOT(num_columns, solver);
    const_SLOT(iters_display_step, solver);
    const_SLOT(is_rank_reachability_prune_enabled, solver);
    const_AUTO(max_iters_limit, maxify(solver->max_iters_limit));
    var_AUTO(iterations_num, solver->iterations_num);

    long next_iterations_display_point =
        ((iters_display_step <= 0)
                ? LONG_MAX
                : (iterations_num + iters_display_step -
                      (iterations_num % iters_display_step)));

    while (solver->queue_len > 0)
    {
        --solver->queue_len;
        const_AUTO(queue_item_copy, solver->queue[solver->queue_len]);
        const_AUTO(state, queue_item_copy.s.unpacked);
        const_AUTO(foundations, state.foundations);

        if (is_rank_reachability_prune_enabled &&
            (bhs_find_rank_reachability__inline(foundations,
                 queue_item_copy.rank_counts.c) != RANK_REACH__SUCCESS))
        {
            continue;
        }

        iterations_num++;

        bool no_cards = TRUE;

        for (int col_idx = 0; col_idx < num_columns; col_idx++)
        {
            const_AUTO(pos, state.heights[col_idx]);
            if (pos)
            {
                no_cards = FALSE;
                const_AUTO(card, solver->board_values[col_idx][pos - 1]);

                if ((foundations == -1) ||
                    (abs(card - foundations) % (MAX_RANK - 1) == 1))
                {
                    perform_move(solver, card, col_idx, &queue_item_copy);
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

fn black_hole_solver_free(
    black_hole_solver_instance_t *instance_proto)
{
    bhs_solver_t *const solver = (bhs_solver_t *)instance_proto;

    bh_solve_hash_free(&(solver->positions));

    if (solver->states_in_solution)
    {
        free(solver->states_in_solution);
        solver->states_in_solution = NULL;
    }

    free(solver->queue);
    solver->queue = NULL;

    free(solver);

    return BLACK_HOLE_SOLVER__SUCCESS;
}

i32 NUM_STATES_INCREMENT = 16;

static void initialize_states_in_solution(bhs_solver_t *solver)
{
    if (solver->states_in_solution)
    {
        return;
    }
    int num_states = 0;
    int max_num_states = NUM_SUITS * NUM_RANKS + 1;

    bhs_solution_state_t *states =
        malloc(sizeof(states[0]) * (size_t)max_num_states);

    states[num_states].packed = (solver->final_state);
    queue_item_unpack(solver, &states[num_states]);

    while (memcmp(&(states[num_states].packed.key), &(solver->init_state.key),
        sizeof(states[num_states].packed.key)))
    {
        if (num_states == max_num_states)
        {
            states = realloc(
                states, sizeof(states[0]) *
                            (size_t)(max_num_states += NUM_STATES_INCREMENT));
        }

        /* Look up the next state in the positions associative array. */
        bh_solve_hash_get(&(solver->positions),
            ((bhs_state_key_value_pair_t *)&(
                states[num_states].packed.value.parent_state)),
            &(states[num_states + 1].packed));
        queue_item_unpack(solver, &states[++num_states]);
    }

    num_states++;

    /* Reverse the list in place. */
    for (int i = 0; i < (num_states >> 1); i++)
    {
        const_AUTO(temp_state, states[i]);
        states[i] = states[num_states - 1 - i];
        states[num_states - 1 - i] = temp_state;
    }

    solver->states_in_solution = states;
    solver->num_states_in_solution = num_states;
    solver->current_state_in_solution_idx = 0;
}

fn black_hole_solver_get_next_move(
    black_hole_solver_instance_t *const instance_proto, int *const col_idx_ptr,
    int *const card_rank_ptr, int *const card_suit_ptr /*  H=0, C=1, D=2, S=3 */
)
{
    bhs_solver_t *const solver = (bhs_solver_t *)instance_proto;

    initialize_states_in_solution(solver);

    if (solver->current_state_in_solution_idx ==
        solver->num_states_in_solution - 1)
    {
        *col_idx_ptr = *card_rank_ptr = *card_suit_ptr = -1;
        return BLACK_HOLE_SOLVER__END;
    }

    {
        const bhs_solution_state_t next_state =
            solver->states_in_solution[++solver->current_state_in_solution_idx];

        const int col_idx = next_state.packed.value.col_idx;
        const int height = next_state.unpacked.heights[col_idx];

        *col_idx_ptr = col_idx;
        solver->sol_foundations_card_rank = *card_rank_ptr =
            solver->board_values[col_idx][height] + 1;
        solver->sol_foundations_card_suit = *card_suit_ptr = suit_char_to_index(
            solver->initial_board_card_strings[col_idx][height][1]);

        return BLACK_HOLE_SOLVER__SUCCESS;
    }
}

extern long __attribute__((pure))
black_hole_solver_get_num_states_in_collection(
    black_hole_solver_instance_t *const instance_proto)
{
    return ((bhs_solver_t *)instance_proto)->num_states_in_collection;
}

extern long __attribute__((pure))
black_hole_solver_get_iterations_num(
    black_hole_solver_instance_t *instance_proto)
{
    return ((bhs_solver_t *)instance_proto)->iterations_num;
}

fn black_hole_solver_get_current_solution_board(
    black_hole_solver_instance_t *instance_proto, char **ptr_to_ret)
{
    bhs_solver_t *const solver = (bhs_solver_t *)instance_proto;

    initialize_states_in_solution(solver);

    *ptr_to_ret = NULL;

    char *const ret = malloc(
        /* 3 bytes per card. */
        (3 * NUM_SUITS * NUM_RANKS) +
        /* newline and a leading ":" per column */
        (2 * BHS__MAX_NUM_COLUMNS) +
        /* For the foundations. */
        20);

    if (ret == NULL)
    {
        return BLACK_HOLE_SOLVER__OUT_OF_MEMORY;
    }

    char *s = ret;

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

    const_SLOT(num_columns, solver);
    for (int col_idx = 0; col_idx < num_columns; col_idx++)
    {
        s += sprintf(s, "%c", ':');
        for (int h = 0; h < next_state.unpacked.heights[col_idx]; h++)
        {
            s += sprintf(
                s, " %s", solver->initial_board_card_strings[col_idx][h]);
        }
        s += sprintf(s, "\n");
    }

    *ptr_to_ret = ret;

    return BLACK_HOLE_SOLVER__SUCCESS;
}
