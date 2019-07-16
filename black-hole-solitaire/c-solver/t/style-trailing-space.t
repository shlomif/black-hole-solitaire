#!/usr/bin/perl

use strict;
use warnings;

use Test::TrailingSpace ();
use Test::More tests => 2;

foreach my $path ( @ENV{qw/FCS_SRC_PATH FCS_BIN_PATH/} )
{
    my $finder = Test::TrailingSpace->new(
        {
            root           => $path,
            filename_regex => qr/./,
            abs_path_prune_re =>
qr#CMakeFiles|(?:CTestTestfile\.cmake\z)|_Inline|lib(?:black_hole_solver|bhs_rank_reach_prune)(?:\.a|\.so)|(?:.*\.tar\.[a-zA-Z0-9_]+)|(?:multi-bhs-solver\z)|(?:black-hole-solve\z)|(?:black-hole-solve-resume-api\z)|(?:\.(?:dll|exe|patch|xcf)\z)#,
        }
    );

    # TEST*2
    $finder->no_trailing_space("No trailing whitespace was found.");
}
