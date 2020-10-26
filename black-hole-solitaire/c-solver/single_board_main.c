// single_board_main.c
// Copyright (C) 2018 Shlomi Fish <shlomif@cpan.org>
//
// Distributed under terms of the Expat license.
#include <solver_common.h>

int main(int argc, char *argv[])
{
    int arg_idx;
    bhs_settings settings = parse_cmd_line(argc, argv, &arg_idx);

    char *filename = NULL;
    if (argc > arg_idx)
    {
        if (strcmp(argv[arg_idx], "-"))
        {
            filename = argv[arg_idx];
        }
    }

    const int ret = solve_filename(filename, &settings);
    solve_free(&settings);
    return ret;
}
