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

const NUM_RANKS:usize = 13;

enum RANK_REACH_VERDICT
{
    RANK_REACH__SUCCESS = 0,
    RANK_REACH__NOT_REACHABLE = 1,
}

fn bhs_find_rank_reachability__inline(foundation: i8, rank_counts: [i8; NUM_RANKS]) -> RANK_REACH_VERDICT
 {
    if foundation < 0
    {
        return RANK_REACH_VERDICT::RANK_REACH__SUCCESS;
    }

    /* The 20 is a margin */
    let mut physical_queue: [i8;NUM_RANKS + 1] = [0;NUM_RANKS+1];

    let mut queue_ptr = 0;

    physical_queue[queue_ptr] = foundation;
    queue_ptr += 1;

    let mut full_ranks_goal = 0;
    for i in rank_counts.iter()
    {
        if i > &0i8
        {
            full_ranks_goal+=1;
        }
    }
    /* Count the foundation - the starting point - in. */
    if rank_counts[foundation as usize] == 0
    {
        full_ranks_goal+=1;
    }

    let mut full_count = 0;

    let mut reached:[bool;NUM_RANKS] = [ false;NUM_RANKS ];

    reached[foundation as usize] = true;
    full_count+=1;

    while (full_count < full_ranks_goal) && (queue_ptr > 0)
    {
        queue_ptr -= 1;
        let rank = physical_queue[queue_ptr as usize];

        const NUM_LINKS:usize= 2;
        const LINKS:[i8;NUM_LINKS] = [ -1, 1 ];
        for o in LINKS.iter()
        {
            let mut offset_rank = o+rank;
            if offset_rank == NUM_RANKS as i8
            {
                offset_rank = 0;
            }
            else if offset_rank == -1
            {
                offset_rank = NUM_RANKS as i8 - 1;
            }

            let o_r = offset_rank as usize;

            if rank_counts[o_r] > 0
            {
                if !reached[o_r]
                {
                    reached[o_r] = true;
                    full_count+=1;
                    physical_queue[ queue_ptr ] = offset_rank;
                    queue_ptr += 1;
                }
            }
        }
    }

    return if full_count == full_ranks_goal { RANK_REACH_VERDICT::RANK_REACH__SUCCESS}
        else { RANK_REACH_VERDICT::RANK_REACH__NOT_REACHABLE };
}
