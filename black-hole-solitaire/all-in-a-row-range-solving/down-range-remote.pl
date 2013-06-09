#!/usr/bin/env perl

use strict;
use warnings;

use LWP::UserAgent;

my $ua = LWP::UserAgent->new;
$ua->timeout(10_000);
$ua->env_proxy;

my $url = 'http://10.0.0.5:3000/id';

my $remote_box = 'lap';
my $remote_dir = "\$HOME/progs/games/black-hole-solitaire/trunk/black-hole-solitaire/range-check";

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

    my $cond = qx{ssh $remote_box 'cd $remote_dir ; if test -e $fn && ! test -z $fn ; then echo 1 ; else echo 0 ; fi'};
    chomp($cond);

    if ($cond)
    {
        die "$id.rs already exists.";
    }
    system(qq{make_pysol_freecell_board.py -F -t "$id" black_hole | }
        .  qq{black-hole-solve --max-iters 4000000 - | }
        .  qq{ssh $remote_box 'cat > $remote_dir/$fn'}
    );
}
