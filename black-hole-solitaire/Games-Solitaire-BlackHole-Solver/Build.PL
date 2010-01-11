use strict;
use warnings;

use lib "./inc";

use Test::Run::Builder;

my $builder = Test::Run::Builder->new(
    module_name         => 'Games::Solitaire::BlackHole::Solver',
    license             => 'mit',
    dist_author         => q{Shlomi Fish <shlomif@iglu.org.il>},
    dist_version_from   => 'lib/Games/Solitaire/BlackHole/Solver.pm',
    requires => {
        'strict' => 0,
        'warnings' => 0,
        'Getopt::Long' => 0,
        'Pod::Usage' => 0,
    },
    build_requires => {
        'Test::More' => 0,
    },
    add_to_cleanup      => [ 'Games-Solitaire-BlackHole-Solver-*' ],
    script_files => ["scripts/black-hole-solve",],
    create_makefile_pl => 'traditional',
    meta_merge =>
    {
        resources =>
        {
            repository => "http://svn.berlios.de/svnroot/repos/fc-solve/black-hole-solitaire/",
            homepage => "http://www.shlomifish.org/open-source/projects/black-hole-solitaire-solver/",
        },
        keywords =>
        [
            "ai",
            "automated",
            "black",
            "card",
            "cards",
            "command line",
            "David Parlett",
            "dfs",
            "games",
            "hole",
            "patience",
            "solitaire",
            "solver",
            "solving",
        ],
    },    
);

$builder->create_build_script();

