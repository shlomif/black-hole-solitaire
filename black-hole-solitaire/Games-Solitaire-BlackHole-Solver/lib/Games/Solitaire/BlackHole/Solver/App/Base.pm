package Games::Solitaire::BlackHole::Solver::App::Base;

use Moo;

extends('Exporter');

has [
    '_board_cards', '_board_values', '_init_foundation', '_talon_cards',
    '_positions',   '_quiet',
] => ( is => 'rw' );
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

    my @lines;
    if ( $filename eq "-" )
    {
        @lines = <STDIN>;
    }
    else
    {
        open my $in, "<", $filename
            or die
            "Could not open $filename for inputting the board lines - $!";
        @lines = <$in>;
        close($in);
    }
    chomp @lines;
    return \@lines;
}

sub _trace_solution
{
    my ( $self, $final_state, $output_handle ) = @_;

    return if $self->_quiet;

    my $state = $final_state;
    my ( $prev_state, $col_idx );

    my @moves;
LOOP:
    while ( ( $prev_state, $col_idx ) = @{ $self->_positions->{$state} } )
    {
        last LOOP if not defined $prev_state;
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

    return;
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

sub _parse_board
{
    my ( $self, $lines ) = @_;

    my $found_line = shift(@$lines);

    my $init_foundation;
    if ( my ($card) = $found_line =~ m{\AFoundations: ($card_re)\z} )
    {
        $init_foundation = $self->_get_rank($card);
    }
    else
    {
        die "Could not match first foundation line!";
    }
    $self->_init_foundation($init_foundation);

    $self->_board_cards( [ map { [ split /\s+/, $_ ] } @$lines ] );
    $self->_board_values(
        [
            map {
                [ map { $self->_get_rank($_) } @$_ ]
            } @{ $self->_board_cards }
        ]
    );
    return;
}

sub _set_up_initial_position
{
    my ( $self, $talon_ptr ) = @_;

    my $init_state = "";

    vec( $init_state, 0, 8 ) = $self->_init_foundation;
    vec( $init_state, 1, 8 ) = $talon_ptr;

    my $board_values = $self->_board_values;
    foreach my $col_idx ( 0 .. $#$board_values )
    {
        vec( $init_state, 4 + $col_idx, 4 ) =
            scalar( @{ $board_values->[$col_idx] } );
    }

    # The values of $positions is an array reference with the 0th key being the
    # previous state, and the 1th key being the column of the move.
    $self->_positions( +{ $init_state => [ undef, undef, 1, 0, ], } );

    return [$init_state];
}

sub _shuffle
{
    my ( $self, $gen, $arr ) = @_;

    my $i = $#$arr;
    while ( $i > 0 )
    {
        my $j = int( $gen->rand( $i + 1 ) );
        if ( $i != $j )
        {
            @$arr[ $i, $j ] = @$arr[ $j, $i ];
        }
        --$i;
    }
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


