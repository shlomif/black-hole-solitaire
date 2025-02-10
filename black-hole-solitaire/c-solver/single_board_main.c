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
        bool dashdash = false;
        if (argv[arg_idx][0] == '-')
        {
            if (strcmp(argv[arg_idx], "--"))
            {
                dashdash = true;
                ++arg_idx;
            }
        }
        if (arg_idx < argc - 1)
        {
            fprintf(stderr,
                "Too many filenames given; only one is accepted.\nStarting "
                "from %s .\n",
                argv[arg_idx]);
            solve_free(&settings);
            return -1;
        }
        if (dashdash || strcmp(argv[arg_idx], "-"))
        {
            if (argv[arg_idx][0] == '-')
            {
                if (!dashdash)
                {
                    fprintf(stderr, "Unknown flag '%s' .\n", argv[arg_idx]);
                    solve_free(&settings);
                    return -1;
                }
            }
            filename = argv[arg_idx];
        }
    }

    bool should_abort;
    const int ret = solve_filename(filename, &settings, &should_abort);
    solve_free(&settings);
    return ret;
}
