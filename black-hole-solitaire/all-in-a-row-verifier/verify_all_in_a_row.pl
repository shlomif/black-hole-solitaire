#!/usr/bin/perl

use strict;
use warnings;

use lib '.';
use Games::Solitaire::Verify::App::Golf ();
Games::Solitaire::Verify::App::Golf->run(
    {
        variant => "all_in_a_row",
    }
);
