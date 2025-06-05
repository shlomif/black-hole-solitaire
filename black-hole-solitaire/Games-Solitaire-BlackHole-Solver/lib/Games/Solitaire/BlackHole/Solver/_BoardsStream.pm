package Games::Solitaire::BlackHole::Solver::_BoardsStream;

use strict;
use warnings;
use autodie;

use 5.014;
use Moo;

has '_boardidx' => ( is => 'rw' );
has '_fh'       => ( is => 'rw', default => 0, );
has '_width'    => ( is => 'rw' );

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

sub _fetch
{
    my ( $self, ) = @_;

    my $s = '';
    read( $self->_fh(), $s, $self->_width );
    my $fn = $self->_board_fn();
    if ( eof( $self->_fh() ) )
    {
        close( $self->_fh() );
        $self->_fh(undef);
        $self->_width(0);
    }
    return ( $fn, $s, );
}

sub _reset
{
    my ( $self, $_fn, $_width, $_boardidx, ) = @_;
    $self->_width($_width);
    $self->_boardidx($_boardidx);
    $self->_my_open($_fn);

    return;
}

1;

__END__
