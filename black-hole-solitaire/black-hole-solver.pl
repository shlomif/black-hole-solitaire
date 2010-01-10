#!/usr/bin/perl

use strict;
use warnings;

my ($filename) = @ARGV;

my @ranks = ("A", 2 .. 9, qw(T J Q K));
my %ranks_to_n = (map { $ranks[$_] => $_ } 0 .. $#ranks);

my $card_re_str = '[' . join("", @ranks) . '][HSCD]';
my $card_re = qr{$card_re_str};

sub get_rank
{
    return $ranks_to_n{substr(shift(), 0, 1)};
}

sub _calc_lines
{
    if ($filename eq "-")
    {
        return [<STDIN>];
    }
    else
    {
        open my $in, "<", $filename
            or die "Could not open $filename for inputting the board lines - $!";
        my @lines = <$in>;
        close($in);
        return \@lines;
    }
}

my @lines = @{_calc_lines()};
chomp(@lines);

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

# The values of %positions is an array reference with the 0th key being the
# previous state, and the 1th key being the column of the move.
my %positions = ($init_state => []);

my @queue = ($init_state);

my %is_good_diff = (map { $_ => 1 } (1, $#ranks));

my $verdict = 0;

QUEUE_LOOP:
while (my $state = pop(@queue))
{
    # The foundation
    my $fnd = vec($state, 0, 8);
    my $no_cards = 1;

    # my @debug_pos;
    foreach my $col_idx (0 .. $#board_values)
    {
        my $pos = vec($state, 4+$col_idx, 2);
        # push @debug_pos, $pos;
        if ($pos)
        {
            $no_cards = 0;

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
                    $positions{$next_s} = [$state, $col_idx];
                    push(@queue, $next_s);
                }
            }
        }
    }
    # print "Checking ", join(",", @debug_pos), "\n";
    if ($no_cards)
    {
        print "Solved!\n";
        _trace_solution($state);
        $verdict = 1;
        last QUEUE_LOOP;
    }
}

if (! $verdict)
{
    print "Unsolved!\n";
}
exit(! $verdict);

sub _trace_solution
{
    my $final_state = shift;

    my $state = $final_state;
    my ($prev_state, $col_idx);

    my @moves;
    while (($prev_state, $col_idx) = @{$positions{$state}})
    {
        push @moves, 
            $board_cards[$col_idx][vec($prev_state, 4+$col_idx, 2)-1]
            ;
    }
    continue
    {
        $state = $prev_state;
    }
    print map { "$_\n" } reverse(@moves);
}
