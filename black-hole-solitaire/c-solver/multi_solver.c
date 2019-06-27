// Copyright (C) 2018 Shlomi Fish <shlomif@cpan.org>
//
// Distributed under terms of the Expat license.
#include "solver_common.h"

int main(int argc, char *argv[])
{
    int arg_idx;
    bhs_settings settings = parse_cmd_line(argc, argv, &arg_idx);

    for (; arg_idx < argc; ++arg_idx)
    {
        char *const filename = argv[arg_idx];
        fprintf(settings.out_fh, "[= Starting file %s =]\n", filename);
        solve_filename(filename, &settings);
        fprintf(settings.out_fh, "[= END of file %s =]\n", filename);
    }
    fflush(settings.out_fh);
    solve_free(&settings);

    return 0;
}
