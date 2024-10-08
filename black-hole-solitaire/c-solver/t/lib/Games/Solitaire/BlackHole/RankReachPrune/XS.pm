package Games::Solitaire::BlackHole::RankReachPrune::XS;

use Config;

use strict;
use warnings;

use Inline (
    C => <<'EOF',
#include "rank_reach_prune.h"

int call_prune(int foundation, AV * rank_counts_av)
{
    bhs_rank_counts rank_counts;
    for (int i = 0; i < NUM_RANKS; ++i)
    {
        SV * * item = av_fetch(rank_counts_av, i, false);
        assert(item);

        rank_counts.c[i] = SvIV(*item);
    }

    return bhs_find_rank_reachability(
        (signed char)foundation,
        &rank_counts
    );
}

EOF
    CLEAN_AFTER_BUILD => 0,
    INC               =>
"-I$ENV{FCS_BIN_PATH} -I$ENV{FCS_SRC_PATH} -I$ENV{FCS_SRC_PATH}/include",
    LIBS => "-L" . $ENV{FCS_BIN_PATH} . " -lbhs_rank_reach_prune",

    # LDDLFLAGS => "$Config{lddlflags} -L$FindBin::Bin -lfcs_delta_states_test",
    # CCFLAGS => "-L$FindBin::Bin -lfcs_delta_states_test",
    # MYEXTLIB => "$FindBin::Bin/libfcs_delta_states_test.so",
    CCFLAGS => "$Config{ccflags} -std=gnu11",
);

sub prune
{
    my ( $class, $foundation, $rank_counts ) = @_;

    return call_prune( $foundation, $rank_counts );
}
1;
