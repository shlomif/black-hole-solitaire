#!/usr/bin/env perl

use strict;
use warnings;

my @next_ids;

my $QUOTA = 20;

sub fetch_id
{
    if (! @next_ids)
    {
        my $line = qx[./get-quota $QUOTA];
        chomp($line);
        my ($start, $finish) = split(/\s+/, $line);
        @next_ids = ($start .. $finish);
    }
    return shift(@next_ids);
}

while ((my $id = fetch_id()) > 0)
{
    print "$id\n";
    my $fn = "$id.rs";

    if (-e $fn && (! -z $fn))
    {
        die "$id.rs already exists.";
    }
    system(qq{make_pysol_freecell_board.py -F -t "$id" black_hole | }
        .  qq{black-hole-solve --max-iters 4000000 - > "$fn"}
    );
}
