// main_common.h
// Copyright (C) 2018 Shlomi Fish <shlomif@cpan.org>
//
// Distributed under terms of the Expat license.
#include "solver_common.h"

int main(int argc, char *argv[])
{
    int arg_idx;
    const bhs_settings settings = parse_cmd_line(argc, argv, &arg_idx);

    for (; arg_idx < argc; ++arg_idx)
    {
        char *const filename = argv[arg_idx];
        fprintf(stdout, "[= Starting file %s =]\n", filename);
        solve_filename(filename, settings);
        fprintf(stdout, "[= END of file %s =]\n", filename);
    }

    return 0;
}
