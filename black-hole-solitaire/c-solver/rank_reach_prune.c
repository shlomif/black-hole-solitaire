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
 * rank_reach_prune.c - find rank-based reachability.
 *
 * See:
 *
 * http://tech.groups.yahoo.com/group/fc-solve-discuss/message/1228
 *
 */

#include "config.h"
#include "rank_reach_prune.h"

#include "bool.h"

enum VERDICTS
{
    SUCCESS = 0,
    NOT_REACHABLE = 1,
};

#define NUM_RANKS 13

static const int LINKS[2] = {-1,1};

DLLEXPORT int bhs_find_rank_reachability(
    signed char foundation,
    const unsigned char * rank_counts
)
{
    int i, link_idx;
    if (foundation < 0)
    {
        return SUCCESS;
    }

    /* The 20 is a margin */
    signed char physical_queue[NUM_RANKS + 20];

    signed char * queue_ptr = physical_queue;

    *(queue_ptr++) = foundation;

    int full_max = 0;
    for (i = 0; i < NUM_RANKS ; i++)
    {
        if (rank_counts[i] > 0)
        {
            full_max++;
        }
    }
    /* Count the foundation - the starting point - in. */
    if (rank_counts[foundation] == 0)
    {
        full_max++;
    }

    int full_count = 0;

    fcs_bool_t reached[NUM_RANKS];

    for (i = 0; i < NUM_RANKS ; i++)
    {
        reached[i] = FALSE;
    }

    MAIN:
    while ((full_count < full_max) && (queue_ptr > physical_queue))
    {
        signed char rank = *(--queue_ptr);

        if (reached[rank])
        {
            continue;
        }

        reached[rank] = TRUE;
        full_count++;

        for (link_idx = 0; link_idx < (sizeof(LINKS)/sizeof(LINKS[0])) ;
            link_idx++)
        {
            int link = LINKS[link_idx];

            signed char offset_rank = (signed char)((rank+link)%NUM_RANKS);

            if (rank_counts[offset_rank] > 0)
            {
                if (! reached[offset_rank])
                {
                    *(queue_ptr++) = offset_rank;
                }
            }
        }
    }

    return ((full_count == full_max) ? SUCCESS : NOT_REACHABLE);
}
