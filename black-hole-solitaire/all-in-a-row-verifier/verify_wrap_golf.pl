#!/usr/bin/perl

use strict;
use warnings;

use lib '.';
use Games::Solitaire::Verify::App::Golf ();
Games::Solitaire::Verify::App::Golf->run(
    {
        variant         => "golf",
        wrap_ranks      => 1,
        queens_on_kings => 1,
    }
);
