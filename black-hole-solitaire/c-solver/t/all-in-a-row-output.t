#!/usr/bin/perl

use strict;
use warnings;

use Test::More tests => 5;
use Test::Differences;

use File::Spec;
use File::Spec::Functions qw( catpath splitpath rel2abs );

use IO::All;

my $bin_dir = catpath( ( splitpath( rel2abs $0 ) )[ 0, 1 ] );

use Test::Trap qw( trap $trap :flow:stderr(systemsafe):stdout(systemsafe):warn);

use Socket qw(:crlf);

sub _normalize_lf
{
    my ($s) = @_;
    $s =~ s#$CRLF#$LF#g;
    return $s;
}

{
    trap
    {
        system('./black-hole-solve',
            '--game', 'all_in_a_row',
            File::Spec->catfile(
                $bin_dir, 'data', '24.all_in_a_row.board.txt'
            )
        );
    };

    # TEST
    ok (! ($trap->exit), "Running the program successfully for board #24.");

    my $expected_output = <<'EOF';
Solved!
Move a card from stack 12 to the foundations

Info: Card moved is AS


====================

Move a card from stack 3 to the foundations

Info: Card moved is KS


====================

Move a card from stack 10 to the foundations

Info: Card moved is QD


====================

Move a card from stack 7 to the foundations

Info: Card moved is JD


====================

Move a card from stack 10 to the foundations

Info: Card moved is TD


====================

Move a card from stack 8 to the foundations

Info: Card moved is 9D


====================

Move a card from stack 12 to the foundations

Info: Card moved is 8H


====================

Move a card from stack 10 to the foundations

Info: Card moved is 7D


====================

Move a card from stack 0 to the foundations

Info: Card moved is 8S


====================

Move a card from stack 0 to the foundations

Info: Card moved is 9H


====================

Move a card from stack 3 to the foundations

Info: Card moved is TS


====================

Move a card from stack 3 to the foundations

Info: Card moved is 9C


====================

Move a card from stack 5 to the foundations

Info: Card moved is 8D


====================

Move a card from stack 6 to the foundations

Info: Card moved is 7C


====================

Move a card from stack 4 to the foundations

Info: Card moved is 6D


====================

Move a card from stack 3 to the foundations

Info: Card moved is 5D


====================

Move a card from stack 5 to the foundations

Info: Card moved is 4D


====================

Move a card from stack 11 to the foundations

Info: Card moved is 3D


====================

Move a card from stack 1 to the foundations

Info: Card moved is 4S


====================

Move a card from stack 1 to the foundations

Info: Card moved is 5C


====================

Move a card from stack 5 to the foundations

Info: Card moved is 6H


====================

Move a card from stack 1 to the foundations

Info: Card moved is 5S


====================

Move a card from stack 2 to the foundations

Info: Card moved is 4H


====================

Move a card from stack 7 to the foundations

Info: Card moved is 3H


====================

Move a card from stack 9 to the foundations

Info: Card moved is 2H


====================

Move a card from stack 12 to the foundations

Info: Card moved is AC


====================

Move a card from stack 11 to the foundations

Info: Card moved is KC


====================

Move a card from stack 6 to the foundations

Info: Card moved is QS


====================

Move a card from stack 9 to the foundations

Info: Card moved is JH


====================

Move a card from stack 7 to the foundations

Info: Card moved is TH


====================

Move a card from stack 0 to the foundations

Info: Card moved is JS


====================

Move a card from stack 2 to the foundations

Info: Card moved is TC


====================

Move a card from stack 10 to the foundations

Info: Card moved is 9S


====================

Move a card from stack 11 to the foundations

Info: Card moved is 8C


====================

Move a card from stack 7 to the foundations

Info: Card moved is 7S


====================

Move a card from stack 9 to the foundations

Info: Card moved is 6S


====================

Move a card from stack 5 to the foundations

Info: Card moved is 7H


====================

Move a card from stack 2 to the foundations

Info: Card moved is 6C


====================

Move a card from stack 1 to the foundations

Info: Card moved is 5H


====================

Move a card from stack 0 to the foundations

Info: Card moved is 4C


====================

Move a card from stack 8 to the foundations

Info: Card moved is 3S


====================

Move a card from stack 11 to the foundations

Info: Card moved is 2S


====================

Move a card from stack 4 to the foundations

Info: Card moved is AD


====================

Move a card from stack 12 to the foundations

Info: Card moved is KD


====================

Move a card from stack 9 to the foundations

Info: Card moved is QH


====================

Move a card from stack 6 to the foundations

Info: Card moved is JC


====================

Move a card from stack 2 to the foundations

Info: Card moved is QC


====================

Move a card from stack 8 to the foundations

Info: Card moved is KH


====================

Move a card from stack 6 to the foundations

Info: Card moved is AH


====================

Move a card from stack 8 to the foundations

Info: Card moved is 2C


====================

Move a card from stack 4 to the foundations

Info: Card moved is 3C


====================

Move a card from stack 4 to the foundations

Info: Card moved is 2D


====================



--------------------
Total number of states checked is 18022.
This scan generated 18056 states.
EOF

    # TEST
    eq_or_diff (
        _normalize_lf($trap->stdout()),
        _normalize_lf($expected_output),
        "Right output from board 24."
    );
}

{
    trap
    {
        system('./black-hole-solve',
            '--game', 'all_in_a_row',
            '--display-boards',
            File::Spec->catfile(
                $bin_dir, 'data', '24.all_in_a_row.board.txt'
            )
        );
    };

    # TEST
    ok (! ($trap->exit), "Exit code for --display-boardsfor board #24.");

    my $expected_prefix = <<'EOF';
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

    my $stdout = $trap->stdout();

    my $got_prefix = substr($stdout, 0, length($expected_prefix));

    # TEST
    eq_or_diff (
        _normalize_lf($got_prefix),
        _normalize_lf($expected_prefix),
        "Right output from board 24 with --display-boards."
    );

    my $expected_stdout = io->file(
        File::Spec->catfile(
            $bin_dir, 'data',
            '24.all_in_a_row.sol-with-display-boards.txt',
        )
    )->slurp;

    # TEST
    eq_or_diff (_normalize_lf($stdout), _normalize_lf($expected_stdout),
        "Complete Right output from board 24 with --display-boards."
    );
}
