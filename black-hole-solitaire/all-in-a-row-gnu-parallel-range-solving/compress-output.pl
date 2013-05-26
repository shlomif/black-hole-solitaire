#!/usr/bin/perl

use strict;
use warnings;

while (<>)
{
    chomp;
    s/\A\d+\t//;
    s/\A([US]\t(\d+))\t\2\z/$1/;
    print "$_\n";
}
