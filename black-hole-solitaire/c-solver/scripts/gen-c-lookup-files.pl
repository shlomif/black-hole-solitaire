#!/usr/bin/perl

use 5.014;
use strict;
use warnings;
use autodie;
use Path::Tiny qw/ path /;

my $FALSE = 0;
my $TRUE  = 1;

my $MAX_RANK              = $ENV{FCS_MAX_RANK} || 13;
my $NUM_SUITS             = 4;
my @SUITS                 = ( 0 .. $NUM_SUITS - 1 );
my @RANKS                 = ( 1 .. $MAX_RANK );
my @PARENT_RANKS          = ( 2 .. $MAX_RANK );
my $MAX_NUM_DECKS         = 1;
my $FCS_POS_BY_RANK_WIDTH = ( $MAX_NUM_DECKS << 3 );

sub make_card
{
    my ( $rank, $suit ) = @_;
    return ( ( $rank << 2 ) | $suit );
}

sub key
{
    my ( $parent, $child ) = @_;
    return "${parent}\t${child}";
}

my $NUM_CHILD_CARDS  = 64;
my $NUM_PARENT_CARDS = make_card( $MAX_RANK, $SUITS[-1] ) + 1;
my @is_king          = ( ($FALSE) x $NUM_PARENT_CARDS );
my %lookup;
my @state_pos = ( map { [ (0) x $NUM_SUITS ] } 0 .. $MAX_RANK );
my @card_pos;
my @positions_by_rank__lookup;
my @pos_by_rank;
my @can_move;

foreach my $wrap_ranks ( 0, 1 )
{
    foreach my $foundations_proto ( 0, @RANKS )
    {
        my $foundations = $foundations_proto - 1;
        foreach my $card_rank_proto (@RANKS)
        {
            my $card = $card_rank_proto - 1;
            my $diff = abs( $card - $foundations );
            $can_move[$wrap_ranks][$foundations_proto][$card] = (
                (
                    ( $foundations == -1 ) || ( $diff == 1
                        || ( $wrap_ranks && ( $diff == $MAX_RANK - 1 ) ) )
                ) ? $TRUE : $FALSE
            );
        }
    }
}

path('board_gen_lookup1.h')->spew_utf8(
    "#pragma once\n",
    'static const size_t offset_by_i[52] = {',
    join(
        ',',
        map {
            my $i   = $_;
            my $col = ( $i & ( 8 - 1 ) );
            3 *
                ( $col * 7 - ( ( $col > 4 ) ? ( $col - 4 ) : 0 ) + ( $i >> 3 ) )
        } 0 .. ( 52 - 1 )
    ),
    "};\n"
);

sub emit
{
    my ( $DECL, $bn, $header_headers, $contents, $types ) = @_;
    $types //= '';

    my $header_fn = "$bn.h";

    path($header_fn)
        ->spew_utf8( "#pragma once\n"
            . join( '', map { qq{#include $_\n} } @$header_headers )
            . $types
            . "extern $DECL;\n" );
    path("$bn.c")
        ->spew_utf8( qq/#include "$header_fn"\n\n$DECL = {/
            . join( ',', @$contents )
            . "};\n" );

    return;
}

emit(
qq#const bool black_hole_solver__can_move[2][@{[$MAX_RANK+1]}][@{[$MAX_RANK]}]#,
    'can_move',
    [ q%<stdbool.h>%, ],
    [
        map {
            my $table = $_;
            '{' . join(
                ',',
                map {
                    my $row = $_;
                    '{'
                        . join( ',', map { $_ ? 'true' : 'false' } @$row ) . '}'
                } @{$table}
                )
                . '}'
        } @can_move
    ],
);
