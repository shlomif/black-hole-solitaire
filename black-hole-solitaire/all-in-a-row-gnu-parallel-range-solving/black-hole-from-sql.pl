#!/usr/bin/perl

use strict;
use warnings;

use DBIx::Simple;
use IO::All;

my $db = DBIx::Simple->connect("dbi:SQLite:dbname=./bhs.sqlite3");

my $fh = io->file("./SUMMARY-no-prune.txt");

{
    my $result = $db->query(
        "SELECT idx, status, num_checked, num_generated FROM bhs_runs");

    $result->bind( my ( $idx, $status, $num_checked, $num_generated ) );

    while ( $result->fetch )
    {
        $fh->append(
            join( "\t", ( $idx, $status, $num_checked, $num_generated ) ),
            "\n" );
    }
}
