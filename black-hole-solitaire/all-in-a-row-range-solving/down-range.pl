#!/usr/bin/env perl

use strict;
use warnings;

use LWP::UserAgent;


my $ua = LWP::UserAgent->new;
$ua->timeout(10_000);
$ua->env_proxy;

my $url = 'http://10.0.0.5:3000/id';

sub fetch_id
{
    my $response = $ua->get($url);
    
    if ($response->is_success())
    {
        my $ret = $response->decoded_content();
        chomp($ret);
        return $ret;
    }
    else
    {
        die $response->status_line();
    }
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
