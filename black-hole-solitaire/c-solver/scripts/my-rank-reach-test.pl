#!/usr/bin/perl

use strict;
use warnings;

use IO::All;

use Games::Solitaire::Verify::Card;

use lib './t/lib';

use Games::Solitaire::BlackHole::RankReachPrune::PP;
use Games::Solitaire::BlackHole::RankReachPrune::XS;

my $SUCCESS = $Games::Solitaire::BlackHole::RankReachPrune::PP::SUCCESS;
my $NOT_REACHABLE =
    $Games::Solitaire::BlackHole::RankReachPrune::PP::NOT_REACHABLE;

sub _r
{
    return Games::Solitaire::Verify::Card->calc_rank(shift) - 1;
}

# TEST:$b=2;
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

my $filename = shift(@ARGV);

my @states = (
    scalar( io->file($filename)->slurp ) =~
        /\[START BOARD\]\n(.*?)\n\[END BOARD\]/gms );

if ( !@states )
{
    die "No states found.";
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

        for my $foundation ( 0 .. ( 13 - 1 ) )
        {
            $test->(
                $foundation, $rank_counts, 'NOT_REACHABLE',
                "$blurb with [start == $foundation]",
            );
        }
    };

    foreach my $backend_var (@backend_specs)
    {
        $backend = $backend_var;

        foreach my $s (@states)
        {
            my $rank_re = qr/[A23456789TJQK]/;

            my $new_s = $s;

            if ( $new_s =~ s/^Foundations: ($rank_re)//ms )
            {
                my $foundation = _r($1);

                my @ranks       = $new_s =~ /($rank_re)/gms;
                my @rank_counts = ( (0) x 13 );
                foreach my $r (@ranks)
                {
                    $rank_counts[ _r($r) ]++;
                }

                if ( $backend->{backend}->( $foundation, \@rank_counts ) !=
                    $SUCCESS )
                {
                    die
"Failed with state: <<<\n$s\n>>>\n and backend $backend->{id}\n";
                }
            }
        }
    }
}
print "All states successful with all backends.\n";

