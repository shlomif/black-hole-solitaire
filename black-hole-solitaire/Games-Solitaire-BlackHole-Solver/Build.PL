use strict;
use warnings;

use lib "./inc";

use Test::Run::Builder;

my $builder = Test::Run::Builder->new(
    module_name         => 'Games::Solitaire::BlackHole::Solver',
    license             => 'mit',
    dist_author         => q{Shlomi Fish <shlomif@iglu.org.il>},
    dist_version_from   => 'lib/Games/Solitaire/BlackHole/Solver.pm',
    build_requires => {
        'Test::More' => 0,
    },
    add_to_cleanup      => [ 'Games-Solitaire-BlackHole-Solver-*' ],
    script_files => ["scripts/black-hole-solve",],
    create_makefile_pl => 'traditional',
);

$builder->create_build_script();
