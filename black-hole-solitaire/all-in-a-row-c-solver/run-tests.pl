#!/usr/bin/perl

use strict;
use warnings;

# use File::Which;
# use File::Basename;
use Cwd;
use FindBin;
use File::Spec;
use File::Copy;
use File::Path;
use Getopt::Long;
use Env::Path;

use File::Spec::Functions qw( catpath splitpath rel2abs );

my $bin_dir = catpath( ( splitpath( rel2abs $0 ) )[ 0, 1 ] );

sub run_tests
{
    my $tests = shift;

    exec("runprove", @$tests);
}

my $tests_glob = "*.{exe,py,t}";

GetOptions(
    '--glob=s' => \$tests_glob,
);

{
    local $ENV{FCS_PATH} = Cwd::getcwd();

    Env::Path->PATH->Prepend(
        File::Spec->catdir(Cwd::getcwd(), "board_gen"),
        File::Spec->catdir(Cwd::getcwd(), "t", "scripts"),
    );

    local $ENV{HARNESS_ALT_INTRP_FILE} =
        File::Spec->rel2abs(
            File::Spec->catdir(
                $bin_dir,
                "t", "config", "alternate-interpreters.yml",
            ),
        )
        ;

    local $ENV{HARNESS_PLUGINS} = 
        "ColorSummary ColorFileVerdicts AlternateInterpreters"
        ;

    if (system("make", "-s"))
    {
        die "make failed";
    }

    # Put the valgrind test last because it takes a long time.
    my @tests =
        sort
        { 
            (($a =~ /valgrind/) <=> ($b =~ /valgrind/))
                ||
            ($a cmp $b)
        }
        glob("$bin_dir/t/$tests_glob")
        ;

    if (! $ENV{FCS_TEST_BUILD})
    {
        @tests = grep { !/build-process/ } @tests;
    }

    {
        # local $ENV{FCS_PATH} = dirname(which("fc-solve"));
        print STDERR "FCS_PATH = $ENV{FCS_PATH}\n";
        run_tests(\@tests);
    }
}


=head1 COPYRIGHT AND LICENSE

Copyright (c) 2000 Shlomi Fish

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.



=cut

