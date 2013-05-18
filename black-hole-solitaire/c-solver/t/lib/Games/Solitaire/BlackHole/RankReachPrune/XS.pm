package Games::Solitaire::BlackHole::RankReachPrune::XS;

use Config;

use strict;
use warnings;

use Inline (
    C => <<'EOF',
#include "rank_reach_prune.h"

#define NUM_RANKS 13

int call_prune(
    int foundation,
    AV * rank_counts_av
    )
{
    int i;
    unsigned char rank_counts[NUM_RANKS];

    for (i = 0; i < NUM_RANKS; i++)
    {
        SV * * item;
        item = av_fetch(rank_counts_av, i, FALSE);
        assert(item);

        rank_counts[i] = SvIV(*item);
    }

    return bhs_find_rank_reachability(
        (signed char)foundation,
        rank_counts
    );
}

EOF
    CLEAN_AFTER_BUILD => 0,
    INC => "-I" . $ENV{FCS_PATH},
    LIBS => "-L" . $ENV{FCS_PATH} . " -lbhs_rank_reach_prune",
    # LDDLFLAGS => "$Config{lddlflags} -L$FindBin::Bin -lfcs_delta_states_test",
    # CCFLAGS => "-L$FindBin::Bin -lfcs_delta_states_test",
    # MYEXTLIB => "$FindBin::Bin/libfcs_delta_states_test.so",
    CCFLAGS => "$Config{ccflags} -std=gnu99",
);

sub prune
{
    my ($class, $foundation, $rank_counts) = @_;

    return call_prune($foundation, $rank_counts);
}
1;
