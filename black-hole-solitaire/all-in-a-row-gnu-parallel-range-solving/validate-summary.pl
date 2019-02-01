#!/usr/bin/perl

use strict;
use warnings;

my $n = 1;

sub is_n
{
    return shift(@_) =~ /\A[1-9][0-9]*/;
}

while ( my $l = <> )
{
    chomp($l);
    my @f = split( /\t/, $l );

    if ( $f[0] ne $n )
    {
        die "Error - wrong num - at Line '$l' at $ARGV : $.!";
    }

    if ( $f[1] !~ m{\A[US]\z} )
    {
        die "Error - wrong verdict - at Line '$l' at $ARGV : $.!";
    }

    if ( !is_n( $f[2] ) )
    {
        die "Error - wrong f[2] - at Line '$l' at $ARGV : $.!";
    }

    if ( !is_n( $f[3] ) )
    {
        die "Error - wrong f[3] - at Line '$l' at $ARGV : $.!";
    }

    if ( $f[2] > $f[3] )
    {
        die "f[2] is bigger than f[3] - at Line '$l' at $ARGV : $.!";
    }
}
continue
{
    $n++;
}
print "OK!\n";
