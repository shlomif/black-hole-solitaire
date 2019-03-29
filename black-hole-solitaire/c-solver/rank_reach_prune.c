// This file is part of Black Hole Solitaire Solver. It is subject to the
// license terms in the COPYING file found in the top-level directory of this
// distribution and at
// https://www.shlomifish.org/open-source/projects/black-hole-solitaire-solver/
// . No part of Black Hole Solitaire Solver, including this file, may be
// copied, modified, propagated, or distributed except according to the terms
// contained in the COPYING file.
//
// Copyright (c) 2010 Shlomi Fish

// rank_reach_prune.c - find rank-based reachability.
//
// See:  http://tech.groups.yahoo.com/group/fc-solve-discuss/message/1228

#include "config.h"
#include "rank_reach_prune.h"

DLLEXPORT enum RANK_REACH_VERDICT bhs_find_rank_reachability(
    const signed char foundation, const bhs_rank_counts *const rank_counts)
{
    return bhs_find_rank_reachability__inline(foundation, rank_counts);
}
