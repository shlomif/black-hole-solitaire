package BHsolver::InlineWrap;

use 5.014;
use strict;
use warnings;

use Config;
use Inline;

use BHsolver::Paths qw( $IS_WIN );

sub import
{
    my ( $self, %args ) = @_;

    my ($pkg) = caller(0);

    my $src  = delete( $args{C} );
    my $libs = delete( $args{l} ) // '';

    my @workaround_for_a_heisenbug =
        ( $IS_WIN ? ( optimize => '-g' ) : () );

    my $ccflags = "$Config{ccflags} -std=gnu11";
    if ($IS_WIN)
    {
        $ccflags =~ s#(^|\s)-[Of][a-zA-Z0-9_\-]*#$1#gms;
    }

    my $KEY                  = 'RINUTILS_INCLUDE_DIR';
    my $RINUTILS_INCLUDE_DIR = exists $ENV{$KEY} ? "-I" . $ENV{$KEY} : '';

    my @inline_params = (
        C    => $src,
        name => $pkg,
        NAME => $pkg,
        INC  =>
"-I$ENV{FCS_BIN_PATH}/include -I$ENV{FCS_SRC_PATH}/rinutils/rinutils/include -I$ENV{FCS_SRC_PATH}/include -I$ENV{FCS_BIN_PATH} -I$ENV{FCS_SRC_PATH} $RINUTILS_INCLUDE_DIR",
        CCFLAGS           => $ccflags,
        CLEAN_AFTER_BUILD => 0,
        LIBS              => "-L$ENV{FCS_BIN_PATH} $libs",
        @workaround_for_a_heisenbug,
        %args,
    );

=begin debug

    if ($IS_WIN)
    {
        require Data::Dumper;
        print STDERR "inline_params = <<<<<\n", Data::Dumper::Dumper( \@inline_params ),
            ">>>>>>\n";
    }

=end debug

=cut

## no critic
    eval "{ package $pkg; Inline->bind(\@inline_params); }";
## use critic

    if ($@)
    {
        die $@;
    }

    return;
}

1;
