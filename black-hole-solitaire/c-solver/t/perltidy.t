#!/usr/bin/perl

use strict;
use warnings;

if ( $ENV{FCS_TEST_SKIP_PERLTIDY} )
{
    require Test::More;
    Test::More::plan( 'skip_all' =>
            "Skipping perltidy test because FCS_TEST_SKIP_PERLTIDY was set" );
}

{
    my $key = "AUTHOR_TESTING";
    if ( not $ENV{$key} )
    {
        require Test::More;
        Test::More::plan(
            'skip_all' => "Skipping perltidy test because $key was not set" );
    }
}

require Test::Code::TidyAll;

Test::Code::TidyAll::tidyall_ok( conf_file => "$ENV{FCS_SRC_PATH}/.tidyallrc",
);
