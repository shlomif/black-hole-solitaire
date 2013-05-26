#!/usr/bin/perl

use strict;
use warnings;

my $n = 1;
while (<>)
{
    chomp;
    my @f = split(/\t/, $_, -1);
    if (@f == 2)
    {
        push @f, $f[-1];
    }
    print join("\t", $n, @f), "\n";
}
continue
{
    $n++;
}
