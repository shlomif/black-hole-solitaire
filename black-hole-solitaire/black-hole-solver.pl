#!/usr/bin/perl

use strict;
use warnings;

use IO::All;

my ($filename) = @ARGV;

my @ranks = ("A", 2 .. 9, qw(T J Q K));
my %ranks_to_n = (map { $ranks[$_] => $_ } 0 .. $#ranks);

my $card_re_str = '[' . join("", @ranks) . '][HSCD]';
my $card_re = qr{$card_re_str};

sub get_rank
{
    return $ranks_to_n{substr(shift(), 0, 1)};
}

my @lines = io->file($filename)->chomp->getlines();

my $found_line = shift(@lines);

my $init_foundation;
if (my ($card) = $found_line =~ m{\AFoundations: ($card_re)\z})
{
    $init_foundation = get_rank($card);
}
else
{
    die "Could not match first foundation line!";
}

my @board_cards = map { [split/\s+/, $_]} @lines;
my @board_values = map { [map { get_rank($_) } @$_ ] } @board_cards;

my $init_state = "";

vec($init_state, 0, 8) = $init_foundation;

foreach my $col_idx (0 .. $#board_values)
{
    vec($init_state, 4+$col_idx, 2) = scalar(@{$board_values[$col_idx]});
}

my %positions = ($init_state => { prev => undef(), });

my @queue = ($init_state);

my %is_good_diff = (map { $_ => 1 } (1, $#ranks));

while (my $state = pop(@queue))
{
    # The foundation
    my $fnd = vec($state, 0, 8);
    my $num_empty = 0;

    # my @debug_pos;
    foreach my $col_idx (0 .. $#board_values)
    {
        my $pos = vec($state, 4+$col_idx, 2);
        # push @debug_pos, $pos;
        if ($pos)
        {
            my $card = $board_values[$col_idx][$pos-1];
            if (exists($is_good_diff{
                ($card - $fnd) % scalar(@ranks)
            }))
            {
                my $next_s = $state;
                vec($next_s, 0, 8) = $card;
                vec($next_s, 4+$col_idx, 2)--;
                if (! exists($positions{$next_s}))
                {
                    $positions{$next_s} = { prev => $state, move => $col_idx };
                    push(@queue, $next_s);
                }
            }
        }
        else
        {
            $num_empty++;
        }
    }
    # print "Checking ", join(",", @debug_pos), "\n";
    if ($num_empty == @board_values)
    {
        print "Reached a final solution!\n";
        exit(0);
    }
}
