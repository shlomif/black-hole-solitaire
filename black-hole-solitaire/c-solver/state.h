
#ifndef BHS_STATE_H
#define BHS_STATE_H


#define MAX_NUM_COLUMNS 17
#define MAX_NUM_CARDS_IN_COL 3


typedef char bhs_card_string_t[3];
typedef char bhs_rank_t;

/* We allocate 2-bits for the length of every column */
#define NUM_DATA_CHARS (MAX_NUM_COLUMNS * 2 / 8 + 1)
typedef struct
{
    unsigned char data[NUM_DATA_CHARS];
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

typedef struct
{
    bhs_state_key_t key;
    bhs_state_value_t value;
} bhs_state_key_value_pair_t;

#endif /*  BHS_STATE_H */
