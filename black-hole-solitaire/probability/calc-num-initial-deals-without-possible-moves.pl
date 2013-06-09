#!/usr/bin/perl

use strict;
use warnings;

use Math::BigInt ":constant", lib => "GMP";

# 1. We allocate 8 positions in the board below the top for the tnk-s:
# npr($num_slots_for_twos_and_kings, $num_slots_for_twos_and_kings - 8)
# 
# The order matters because each permutation is different.

# 2. Next we must allocate the remaining $num_cards_in_layout - 8 cards.
# - How?
# fact($num_cards_in_layout - 8)

sub fact
{
    my $n = shift;

    return $n->copy->bfac();
}

sub npr
{
    my ($n, $r) = @_;

    return (fact($n) / fact($r));
}

sub ncr
{
    my ($n, $r) = @_;

    return npr($n,$r) / fact($n-$r);
}

my $rank = 13;

my $num_slots_for_twos_and_kings = 2 * 17;

# tnk = twos and kings
my $num_tnk = 8;

my $num_cards_in_layout = $rank * 4 - 1;

my $num_non_tnk = $num_cards_in_layout - $num_tnk;

my $places_alloc_for_tnk = ncr($num_slots_for_twos_and_kings, $num_tnk);

# my $tnk_org = fact($num_tnk);

# my $non_tnk_org = fact($num_non_tnk);

my $result = npr($num_slots_for_twos_and_kings, $num_slots_for_twos_and_kings - $num_tnk)
    * fact($num_cards_in_layout - $num_tnk);

my $total = fact(53);

print "Result = ", $result, "\n";

print "Total  = ", $total, "\n";

print "Ratio  = ", $total/$result, "\n";
