package Games::Solitaire::BlackHole::Solver::_BoardsStream;

use strict;
use warnings;

use 5.014;
use Moo;

has '_boardidx' => ( is => 'rw' );
has '_fh'       => ( is => 'rw', default => 0, );
has '_width'    => ( is => 'ro' );

sub _board_fn
{
    my ( $self, ) = @_;

    my $ret = sprintf( "deal%d", $self->_boardidx() );
    $self->_boardidx( 1 + $self->_boardidx() );

    return $ret;
}

sub _my_open
{
    my ( $self, $fn ) = @_;

    open my $read_fh, "<", $fn;
    $self->_fh($read_fh);

    return;
}

1;

__END__
