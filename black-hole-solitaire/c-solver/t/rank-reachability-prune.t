#!/usr/bin/perl

use strict;
use warnings;
use autodie;

use Test::More tests => 120;

use lib './t/lib';

use Games::Solitaire::BlackHole::RankReachPrune::PP ();
use Games::Solitaire::BlackHole::RankReachPrune::XS ();

my $SUCCESS = $Games::Solitaire::BlackHole::RankReachPrune::PP::SUCCESS;
my $NOT_REACHABLE =
    $Games::Solitaire::BlackHole::RankReachPrune::PP::NOT_REACHABLE;

# TEST:$num_backends=2;
my @backend_specs = (
    {
        id      => "perl",
        backend => sub {
            my ( $foundation, $rank_counts ) = @_;
            return Games::Solitaire::BlackHole::RankReachPrune::PP->prune(
                $foundation, $rank_counts, );
        },
    },
    {
        id      => "c_backend",
        backend => sub {
            my ( $foundation, $rank_counts ) = @_;
            return Games::Solitaire::BlackHole::RankReachPrune::XS->prune(
                $foundation, $rank_counts, );
        },
    }
);

sub _verdict_to_s
{
    my $v = shift;
    return +(
          ( $v eq $SUCCESS )       ? 'SUCCESS'
        : ( $v eq $NOT_REACHABLE ) ? 'NOT_REACHABLE'
        :                            ( die "Unknown Verdict '$v'." )
    );
}

{
    my $backend;

    my $test = sub {
        local $Test::Builder::Level = $Test::Builder::Level + 1;
        my ( $foundation, $rank_counts, $verdict, $blurb ) = @_;

        is(
            _verdict_to_s( $backend->{backend}->( $foundation, $rank_counts ) ),
            $verdict,
            "$backend->{id} - $blurb"
        );
    };

    # Test the unreachability for all indexes
    my $test_unreachable_for_all = sub {
        my ( $rank_counts, $blurb ) = @_;

        local $Test::Builder::Level = $Test::Builder::Level + 1;

        # TEST:$unr=13;
        for my $foundation ( 0 .. ( 13 - 1 ) )
        {
            $test->(
                $foundation, $rank_counts, 'NOT_REACHABLE',
                "$blurb with [start == $foundation]",
            );
        }
    };

    # TEST:FILTER(MULT($num_backends))
    foreach my $backend_var (@backend_specs)
    {
        $backend = $backend_var;

        # TEST
        $test->(
            8,         [ 2, 3, 2, 1, 1, 2, 2, 1, 0, 0, 1, 2, 2 ],
            'SUCCESS', "Failure in C backend No. 1",
        );

        # TEST
        $test->(
            -1,        [ (1) x 13 ],
            'SUCCESS', "Always true on foundation of -1."
        );

        # TEST
        $test->( 0, [ (1) x 13 ], 'SUCCESS', "All is all-ranks-reachable." );

        # TEST
        $test->(
            0,         [ ( (1) x 10 ), ( (0) x 3 ) ],
            'SUCCESS', "First 10 ranks",
        );

        # TEST
        $test->(
            0,               [ ( (1) x 10 ), 0, 1, 0 ],
            'NOT_REACHABLE', "Unreachable island.",
        );

        # TEST
        $test->(
            4,         [ 0, 0, 0, 2, 1, 2, 0, 0, 0, 0, 0, 0, 0, ],
            'SUCCESS', "two reachable segment",
        );

        # TEST
        $test->(
            2,         [ 0, 0, 0, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, ],
            'SUCCESS', "two reachable segment",
        );

        # TEST
        $test->(
            2,               [ 0, 0, 0, 2, 1, 0, 0, 1, 0, 0, 0, 0, 0, ],
            'NOT_REACHABLE', "One island.",
        );

        # TEST:FILTER(MULT($unr))
        # TEST
        $test_unreachable_for_all->(
            [ 0, 0, 0, 2, 1, 0, 0, 1, 0, 1, 0, 0, 0, ],
            "Two islands.",
        );

        # TEST
        $test_unreachable_for_all->(
            [ 3, 0, 0, 2, 1, 0, 0, 1, 0, 1, 0, 0, 0, ],
            "Three islands",
        );

        # TEST
        $test_unreachable_for_all->(
            [ 3, 0, 0, 2, 1, 0, 0, 1, 0, 1, 0, 0, 0, ],
            "Three islands",
        );

        # TEST
        $test_unreachable_for_all->(
            [ 0, 1, 1, 1, 0, 0, 0, 0, 2, 2, 0, 0, 0, ],
            "Two separated islands.",
        );

        # TEST:ENDFILTER()

    }

    # TEST:ENDFILTER()
}

