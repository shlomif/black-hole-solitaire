package Games::Solitaire::BlackHole::RankReachPrune::PP;

use strict;
use warnings;

our $SUCCESS = 0;
our $NOT_REACHABLE = 1;

sub prune
{
    my ($class, $foundation, $rank_counts) = @_;

    if ($foundation < 0)
    {
        return $SUCCESS;
    }

    return $NOT_REACHABLE;
}

1;
