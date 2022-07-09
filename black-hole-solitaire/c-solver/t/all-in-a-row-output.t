#!/usr/bin/perl

use strict;
use warnings;

use Test::More tests => 5;
use Test::Differences qw/ eq_or_diff /;

use Path::Tiny qw/ path /;

my $bin_dir   = path(__FILE__)->parent->absolute;
my $data_dir  = $bin_dir->child('data');
my $texts_dir = $data_dir->child('texts');

my $exit_code;

sub mysys
{
    return $exit_code = system(@_);
}

use Dir::Manifest        ();
use Dir::Manifest::Slurp qw/ as_lf /;
my $mani = Dir::Manifest->new(
    {
        manifest_fn => $texts_dir->child('list.txt'),
        dir         => $texts_dir->child('texts'),
    }
);
use Test::Trap qw( trap $trap :flow:stderr(systemsafe):stdout(systemsafe):warn);

{
    trap
    {
        mysys( './black-hole-solve', '--game', 'all_in_a_row',
            $data_dir->child('24.all_in_a_row.board.txt'),
        );
    };

    # TEST
    ok( !($exit_code), "Running the program successfully for board #24." );

    # TEST
    eq_or_diff(
        as_lf( $trap->stdout() ),
        $mani->text( "24.all_in_a_row.sol.txt", { lf => 1 } ),
        "Right output from board 24."
    );
}

{
    trap
    {
        mysys( './black-hole-solve', '--game', 'all_in_a_row',
            '--display-boards', $data_dir->child('24.all_in_a_row.board.txt'),
        );
    };

    # TEST
    ok( !($exit_code), "Exit code for --display-boards for board #24." );

    my $expected_prefix = as_lf(<<'EOF');
Solved!

[START BOARD]
Foundations: -
: 4C JS 9H 8S
: 5H 5S 5C 4S
: QC 6C TC 4H
: 5D 9C TS KS
: 2D 3C AD 6D
: 7H 6H 4D 8D
: AH JC QS 7C
: 7S TH 3H JD
: 2C KH 3S 9D
: QH 6S JH 2H
: 9S 7D TD QD
: 2S 8C KC 3D
: KD AC 8H AS
[END BOARD]


Move a card from stack 12 to the foundations

Info: Card moved is AS


====================


[START BOARD]
Foundations: AS
: 4C JS 9H 8S
: 5H 5S 5C 4S
: QC 6C TC 4H
: 5D 9C TS KS
: 2D 3C AD 6D
: 7H 6H 4D 8D
: AH JC QS 7C
: 7S TH 3H JD
: 2C KH 3S 9D
: QH 6S JH 2H
: 9S 7D TD QD
: 2S 8C KC 3D
: KD AC 8H
[END BOARD]


Move a card from stack 3 to the foundations

Info: Card moved is KS


====================


[START BOARD]
Foundations: KS
: 4C JS 9H 8S
: 5H 5S 5C 4S
: QC 6C TC 4H
: 5D 9C TS
: 2D 3C AD 6D
: 7H 6H 4D 8D
: AH JC QS 7C
: 7S TH 3H JD
: 2C KH 3S 9D
: QH 6S JH 2H
: 9S 7D TD QD
: 2S 8C KC 3D
: KD AC 8H
[END BOARD]


Move a card from stack 10 to the foundations

Info: Card moved is QD

EOF

    my $stdout = as_lf( $trap->stdout() );

    my $got_prefix = substr( $stdout, 0, length($expected_prefix) );

    # TEST
    eq_or_diff( as_lf($got_prefix), as_lf($expected_prefix),
        "Right output from board 24 with --display-boards." );

    my $expected_stdout =
        $mani->text( '24.all_in_a_row.sol-with-display-boards.txt',
        { lf => 1 } );

    # TEST
    eq_or_diff( as_lf($stdout), $expected_stdout,
        "Complete Right output from board 24 with --display-boards." );
}
