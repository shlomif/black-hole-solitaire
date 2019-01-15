package BHsolver::Paths;

use 5.014;
use strict;
use warnings;

use parent 'Exporter';

our @EXPORT_OK = qw/ $IS_WIN /;

our $IS_WIN = ( $^O eq "MSWin32" );

1;
