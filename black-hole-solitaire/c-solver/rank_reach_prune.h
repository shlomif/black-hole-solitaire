// This file is part of Black Hole Solitaire Solver. It is subject to the
// license terms in the COPYING file found in the top-level directory of this
// distribution and at
// https://www.shlomifish.org/open-source/projects/black-hole-solitaire-solver/
// . No part of Black Hole Solitaire Solver, including this file, may be
// copied, modified, propagated, or distributed except according to the terms
// contained in the COPYING file.
//
// Copyright (c) 2010 Shlomi Fish

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <black-hole-solver/fcs_dllexport.h>

#define NUM_RANKS 13

enum RANK_REACH_VERDICT
{
    RANK_REACH__SUCCESS = 0,
    RANK_REACH__NOT_REACHABLE = 1,
};

typedef struct
{
    uint8_t c[NUM_RANKS];
} bhs_rank_counts;

DLLEXPORT enum RANK_REACH_VERDICT bhs_find_rank_reachability(
    const signed char foundation, const bhs_rank_counts *const rank_counts);

static inline enum RANK_REACH_VERDICT bhs_find_rank_reachability__inline(
    const signed char foundation, const bhs_rank_counts *const rank_counts)
{
    if (foundation < 0)
    {
        return RANK_REACH__SUCCESS;
    }

    /* The 20 is a margin */
    signed char physical_queue[NUM_RANKS + 1];

    signed char *queue_ptr = physical_queue;

    *(queue_ptr++) = foundation;

    uint32_t full_ranks_goal = 0;
    for (size_t i = 0; i < NUM_RANKS; ++i)
    {
        if (rank_counts->c[i] > 0)
        {
            ++full_ranks_goal;
        }
    }
    /* Count the foundation - the starting point - in. */
    if (rank_counts->c[foundation] == 0)
    {
        ++full_ranks_goal;
    }

    uint32_t full_count = 1;
    bool reached[NUM_RANKS] = {false};
    reached[foundation] = true;

    while ((full_count < full_ranks_goal) && (queue_ptr > physical_queue))
    {
        const signed char rank = *(--queue_ptr);

        static const int LINKS[2] = {-1, 1};
        for (size_t link_idx = 0; link_idx < (sizeof(LINKS) / sizeof(LINKS[0]));
            link_idx++)
        {
            signed char offset_rank = (signed char)(rank + LINKS[link_idx]);

            if (offset_rank == NUM_RANKS)
            {
                offset_rank = 0;
            }
            else if (offset_rank == -1)
            {
                offset_rank = NUM_RANKS - 1;
            }

            if (rank_counts->c[offset_rank] > 0)
            {
                if (!reached[offset_rank])
                {
                    reached[offset_rank] = true;
                    ++full_count;
                    *(queue_ptr++) = offset_rank;
                }
            }
        }
    }

    return ((full_count == full_ranks_goal) ? RANK_REACH__SUCCESS
                                            : RANK_REACH__NOT_REACHABLE);
}

#ifdef __cplusplus
}
#endif
