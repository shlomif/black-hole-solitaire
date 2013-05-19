#!/usr/bin/perl

use strict;
use warnings;

use IO::All;

use lib './t/lib';

use Games::Solitaire::BlackHole::RankReachPrune::PP;
use Games::Solitaire::BlackHole::RankReachPrune::XS;

my $SUCCESS = $Games::Solitaire::BlackHole::RankReachPrune::PP::SUCCESS;
my $NOT_REACHABLE = $Games::Solitaire::BlackHole::RankReachPrune::PP::NOT_REACHABLE;

# TEST:$b=2;
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
    },
    {
        id => "c_backend",
        backend => sub {
            my ($foundation, $rank_counts) = @_;
            return Games::Solitaire::BlackHole::RankReachPrune::XS->
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

    # Test the unreachability for all indexes
    my $test_unreachable_for_all = sub {
        my ($rank_counts, $blurb) = @_;

        local $Test::Builder::Level = $Test::Builder::Level + 1;

        for my $foundation (0 .. (13-1))
        {
            $test->(
                $foundation,
                $rank_counts,
                'NOT_REACHABLE',
                "$blurb with [start == $foundation]",
            );
        }
    };

    foreach my $backend_var (@backend_specs)
    {
        $backend = $backend_var;
    }
}

