#pragma once

#include "min_and_max.h"

#define BHS__ALL_IN_A_ROW__NUM_COLUMNS 13
#define BHS__ALL_IN_A_ROW__MAX_NUM_CARDS_IN_COL 4
#define BHS__ALL_IN_A_ROW__BITS_PER_COL 3

#define BHS__GOLF__NUM_COLUMNS 7
#define BHS__GOLF__MAX_NUM_CARDS_IN_COL 5
#define BHS__GOLF__BITS_PER_COL 3

#define BHS__BLACK_HOLE__NUM_COLUMNS 17
#define BHS__BLACK_HOLE__MAX_NUM_CARDS_IN_COL 3
#define BHS__BLACK_HOLE__BITS_PER_COL 2

#define BHS_CARD_STRING_LEN 2
typedef char bhs_card_string_t[BHS_CARD_STRING_LEN + 1];
typedef char bhs_rank_t;

#define BHS__MAX_NUM_COLUMNS                                                   \
    (max(BHS__ALL_IN_A_ROW__NUM_COLUMNS, BHS__BLACK_HOLE__NUM_COLUMNS))
#define BHS__MAX_NUM_CARDS_IN_COL                                              \
    (max(BHS__ALL_IN_A_ROW__MAX_NUM_CARDS_IN_COL,                              \
        BHS__BLACK_HOLE__MAX_NUM_CARDS_IN_COL))

#define BHS__NUM_BITS_PER_CHAR 8
/* We allocate 4-bits for the length of every column */
#define BHS__ALL_IN_A_ROW__NUM_DATA_CHARS                                      \
    (BHS__ALL_IN_A_ROW__NUM_COLUMNS * BHS__ALL_IN_A_ROW__BITS_PER_COL /        \
            BHS__NUM_BITS_PER_CHAR +                                           \
        2)
#define BHS__BLACK_HOLE__NUM_DATA_CHARS                                        \
    (BHS__BLACK_HOLE__NUM_COLUMNS * BHS__BLACK_HOLE__BITS_PER_COL /            \
            BHS__NUM_BITS_PER_CHAR +                                           \
        2)

#define BHS__NUM_DATA_CHARS                                                    \
    (max(BHS__ALL_IN_A_ROW__NUM_DATA_CHARS, BHS__BLACK_HOLE__BITS_PER_COL))

typedef struct
{
    unsigned char data[BHS__NUM_DATA_CHARS];
    bhs_rank_t foundations;
} bhs_state_key_t;

typedef char bhs_col_idx_t;

typedef struct
{
    /* The index of the column that was changed. */
    bhs_col_idx_t col_idx;
    bhs_rank_t prev_foundation;
} bhs_state_value_t;

typedef struct
{
    bhs_state_key_t key;
    bhs_state_value_t value;
} bhs_state_key_value_pair_t;
