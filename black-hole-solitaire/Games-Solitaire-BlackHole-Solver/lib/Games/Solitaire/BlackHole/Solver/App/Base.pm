package Games::Solitaire::BlackHole::Solver::App::Base;

use Moo;

extends('Exporter');

has [ '_board_cards', '_talon_cards', '_positions' ] => ( is => 'rw' );
our %EXPORT_TAGS = ( 'all' => [qw($card_re)] );
our @EXPORT_OK   = ( @{ $EXPORT_TAGS{'all'} } );

my @ranks      = ( "A", 2 .. 9, qw(T J Q K) );
my %ranks_to_n = ( map { $ranks[$_] => $_ } 0 .. $#ranks );

sub _RANK_KING
{
    return $ranks_to_n{'K'};
}

my $card_re_str = '[' . join( "", @ranks ) . '][HSCD]';
our $card_re = qr{$card_re_str};

sub _get_rank
{
    shift;
    return $ranks_to_n{ substr( shift(), 0, 1 ) };
}

sub _calc_lines
{
    my $self     = shift;
    my $filename = shift;

    if ( $filename eq "-" )
    {
        return [<STDIN>];
    }
    else
    {
        open my $in, "<", $filename
            or die
            "Could not open $filename for inputting the board lines - $!";
        my @lines = <$in>;
        close($in);
        return \@lines;
    }
}

sub _trace_solution
{
    my ( $self, $final_state, $output_handle ) = @_;

    my $state = $final_state;
    my ( $prev_state, $col_idx );

    my @moves;
    while ( ( $prev_state, $col_idx ) = @{ $self->_positions->{$state} } )
    {
        push @moves,
            (
            ( $col_idx == @{ $self->_board_cards } )
            ? "Deal talon " . $self->_talon_cards->[ vec( $prev_state, 1, 8 ) ]
            : $self->_board_cards->[$col_idx]
                [ vec( $prev_state, 4 + $col_idx, 4 ) - 1 ]
            );
    }
    continue
    {
        $state = $prev_state;
    }
    print {$output_handle} map { "$_\n" } reverse(@moves);
}

sub _my_exit
{
    my ( $self, $verdict, $output_handle, $output_fn ) = @_;

    if ( !$verdict )
    {
        print {$output_handle} "Unsolved!\n";
    }

    if ( defined($output_fn) )
    {
        close($output_handle);
    }

    exit( !$verdict );

    return;
}

1;

__END__

=head1 NAME

Games::Solitaire::BlackHole::Solver::App::Base - base class.

=head1 METHODS

=head2 new

For internal use.

=cut


