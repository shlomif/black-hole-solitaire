#!/usr/bin/perl

use strict;
use warnings;

use Test::TrailingSpace ();
use Test::More tests => 2;

foreach my $path (@ENV{qw/FCS_SRC_PATH FCS_PATH/})
{
    my $finder = Test::TrailingSpace->new(
        {
            root => $path,
            filename_regex => qr/./,
            abs_path_prune_re => qr#CMakeFiles|(?:CTestTestfile\.cmake\z)|_Inline|libblack_hole_solver(?:\.a|\.so)|(?:\.(?:xcf|patch)\z)#,
        }
    );

    # TEST*2
    $finder->no_trailing_space("No trailing whitespace was found.")
}
