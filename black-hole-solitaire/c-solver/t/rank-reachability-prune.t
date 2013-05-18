#!/usr/bin/perl

use strict;
use warnings;

use Test::More tests => 9;

use lib './t/lib';

use Games::Solitaire::BlackHole::RankReachPrune::PP;

my $SUCCESS = $Games::Solitaire::BlackHole::RankReachPrune::PP::SUCCESS;
my $NOT_REACHABLE = $Games::Solitaire::BlackHole::RankReachPrune::PP::NOT_REACHABLE;

# TEST:$b=1;
my @backend_specs =
(
    {
        id => "perl",
        backend => sub {
            my ($foundation, $rank_counts) = @_;
            return Games::Solitaire::BlackHole::RankReachPrune::PP->
                prune(
                    $foundation,
                    $rank_counts,
                );
        },
    }
);

sub _verdict_to_s
{
    my $v = shift;
    return +(
          ($v eq $SUCCESS) ? 'SUCCESS'
        : ($v eq $NOT_REACHABLE) ? 'NOT_REACHABLE'
        : (die "Unknown Verdict '$v'.")
    );
}

{
    my $backend;

    my $test = sub {
        local $Test::Builder::Level = $Test::Builder::Level + 1;
        my ($foundation, $rank_counts, $verdict, $blurb) = @_;

        is (
            _verdict_to_s(
                $backend->{backend}->($foundation, $rank_counts)
            ),
            $verdict,
            "$backend->{id} - $blurb"
        );
    };

    # TEST:$c=0;
    foreach my $backend_var (@backend_specs)
    {
        $backend = $backend_var;

        # TEST:$c++;
        $test->(-1, [(1) x 13], 'SUCCESS', "Always true on foundation of -1.");

        # TEST:$c++;
        $test->(0, [(1) x 13], 'SUCCESS', "All is all-ranks-reachable.");

        # TEST:$c++;
        $test->(
            0, [((1) x 10),((0) x 3)],
            'SUCCESS',
            "First 10 ranks",
        );

        # TEST:$c++;
        $test->(
            0, [((1) x 10),0,1,0],
            'NOT_REACHABLE',
            "Unreachable island.",
        );

        # TEST:$c++;
        $test->(
            4, [0,0,0,2,1,2,0,0,0,0,0,0,0,],
            'SUCCESS',
            "two reachable segment",
        );

        # TEST:$c++;
        $test->(
            2, [0,0,0,2,1,0,0,0,0,0,0,0,0,],
            'SUCCESS',
            "two reachable segment",
        );

        # TEST:$c++;
        $test->(
            2, [0,0,0,2,1,0,0,1,0,0,0,0,0,],
            'NOT_REACHABLE',
            "One island.",
        );

        # TEST:$c++;
        $test->(
            2, [0,0,0,2,1,0,0,1,0,1,0,0,0,],
            'NOT_REACHABLE',
            "Two islands.",
        );

        # TEST:$c++;
        $test->(
            2, [3,0,0,2,1,0,0,1,0,1,0,0,0,],
            'NOT_REACHABLE',
            "Three islands.",
        );
    }

    # TEST:$per_backend_tests=$c;

    # TEST*$per_backend_tests*$b
}

