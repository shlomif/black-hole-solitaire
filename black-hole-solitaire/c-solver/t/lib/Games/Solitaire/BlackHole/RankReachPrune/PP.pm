package Games::Solitaire::BlackHole::RankReachPrune::PP;

use strict;
use warnings;

our $SUCCESS = 0;
our $NOT_REACHABLE = 1;

my $NUM_RANKS = 13;

sub prune
{
    my ($class, $foundation, $rank_counts) = @_;

    if ($foundation < 0)
    {
        return $SUCCESS;
    }

    my $FALSE = 0;
    my $TRUE = 1;

    my @queue = ($foundation);

    my $full_max = grep { $_ > 0 } @$rank_counts;

    my $full_count = 0;

    my @reached = (($FALSE) x @$rank_counts);

    MAIN:
    while (($full_count < $full_max) || @queue)
    {
        my $rank = pop(@queue);

        if ($reached[$rank])
        {
            next MAIN;
        }

        $reached[$rank] = $TRUE;
        $full_count++;

        for my $link (-1, 1)
        {
            my $offset_rank = $rank+$link;

            if ($offset_rank == $NUM_RANKS)
            {
                $offset_rank = 0;
            }
            elsif ($offset_rank == -1)
            {
                $offset_rank = $NUM_RANKS-1;
            }

            if ($rank_counts->[$offset_rank] > 0)
            {
                if (! $reached[$offset_rank])
                {
                    push @queue, $offset_rank;
                }
            }
        }
    }

    return (($full_count == $full_max) ? $SUCCESS : $NOT_REACHABLE);
}

1;
