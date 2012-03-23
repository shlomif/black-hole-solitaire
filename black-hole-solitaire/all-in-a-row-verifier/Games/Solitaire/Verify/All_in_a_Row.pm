package Games::Solitaire::Verify::All_in_a_Row;

use strict;
use warnings;

use Carp;

use Games::Solitaire::Verify::Column;
use Games::Solitaire::Verify::Card;
use Games::Solitaire::Verify::Freecells;

use base 'Games::Solitaire::Verify::Base';

__PACKAGE__->mk_acc_ref([qw(_columns _foundation)]);

sub _init
{
    my ($self, $args) = @_;

    my $board_string = $args->{board_string};

    my @lines = split(/\n/, $board_string);

    my $foundation_str = shift(@lines);

    if ($foundation_str ne "Foundations: -")
    {
        Carp::confess( "Foundations str is '$foundation_str'" );
    }

    $self->_foundation(
        Games::Solitaire::Verify::Freecells->new({ count => 1 })
    );

    $self->_columns(
        [
            map
            {
                Games::Solitaire::Verify::Column->new(
                    {
                        string => ": $_",
                    }
                )
            } @lines
        ]
    );

    return;
}

sub process_solution
{
    my ($self, $next_line_iter) = @_;

    my $line_num = 0;

    my $get_line = sub {
        my $ret = $next_line_iter->();
        $line_num++;
        return ($ret, $line_num);
    };

    my $assert_empty_line = sub {
        my ($s, $line_idx) = $get_line->();
        
        if ($s ne '')
        {
            die "Line '$line_idx' is not empty, but '$s'";
        }
        
        return;
    };

    my ($l, $first_l) = $get_line->();

    if ($l ne "Solved!")
    {
        die "First line is '$l' instead of 'Solved!'";
    }

    # As many moves as the number of cards.
    for my $move_idx (0 .. (13 * 4 - 1))
    {
        my ($move_line, $move_line_idx) = $get_line->();

        if ($move_line !~ m/\AMove a card from stack (\d+) to the foundations\z/)
        {
            die "Incorrect format for move line no. $move_line_idx - '$move_line'";
        }

        my $col_idx = $1;

        if (($col_idx < 0) or ($col_idx >= 13))
        {
            die "Invalid column index '$col_idx' at $move_line_idx";
        }

        $assert_empty_line->();

        my ($info_line, $info_line_idx) = $get_line->();

        if ($info_line !~ m/\AInfo: Card moved is ([A23456789TJQK][HCDS])\z/)
        {
            die "Invalid format for info line no. $info_line_idx - '$info_line'";
        }

        my $moved_card_str = $1;

        $assert_empty_line->();
        $assert_empty_line->();

        my ($sep_line, $sep_line_idx) = $get_line->();

        if ($sep_line !~ m/\A=+\z/)
        {
            die "Invalid format for separator line no. $sep_line_idx - '$sep_line'";
        }

        $assert_empty_line->();

        my $col = $self->_columns->[$col_idx];
        my $top_card = $col->top();
        my $top_card_moved_str = $top_card->to_string();

        if ($top_card_moved_str ne $moved_card_str)
        {
            die "Card moved should be '$top_card_moved_str', but the info says it is '$moved_card_str' at line $info_line_idx";
        }

        if (defined($self->_foundation->cell(0)))
        {
            my $found_card = $self->_foundation->cell(0);

            my $delta = abs ( $top_card->rank() - $found_card->rank() );
            if ( not ($delta == 1 or $delta == (13-1)) )
            {
                die "Cannot put " . $top_card->to_string() . " in the foundations that contain " . $found_card->to_string();
            }
        }
     
        # Now just perform the move.
        $self->_foundation->assign(0, $col->pop())
    }

    return;
}

1;

