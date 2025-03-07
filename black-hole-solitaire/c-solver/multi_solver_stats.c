// Copyright (C) 2018 Shlomi Fish <shlomif@cpan.org>
//
// Distributed under terms of the Expat license.
#include <solver_common.h>

static inline int output_stats__solve_file(
    const char *const filename, bhs_settings *const settings_ptr)
{
#define settings (*settings_ptr)
    FILE *fh = stdin;
    if (filename)
    {
        fh = fopen(filename, "rt");
        if (!fh)
        {
            fprintf(stderr, "Cannot open '%s' for reading!\n", filename);
            return -1;
        }
    }
    char board[MAX_LEN_BOARD_STRING];
    fread(board, sizeof(board[0]), MAX_LEN_BOARD_STRING, fh);

    if (filename)
    {
        fclose(fh);
    }

    board[MAX_LEN_BOARD_STRING - 1] = '\0';

    int error_line_num;
    const enum GAME_TYPE game_type = settings.game_type;
    const unsigned num_columns =
        ((game_type == GAME__BH)        ? BHS__BLACK_HOLE__NUM_COLUMNS
            : (game_type == GAME__GOLF) ? BHS__GOLF__NUM_COLUMNS
                                        : BHS__ALL_IN_A_ROW__NUM_COLUMNS);
#define solver (settings_ptr->the_solver)
    if (black_hole_solver_read_board(solver, board, &error_line_num,
            num_columns,
            ((game_type == GAME__BH) ? BHS__BLACK_HOLE__MAX_NUM_CARDS_IN_COL
                : (game_type == GAME__GOLF)
                    ? BHS__GOLF__MAX_NUM_CARDS_IN_COL
                    : BHS__ALL_IN_A_ROW__MAX_NUM_CARDS_IN_COL),
            ((game_type == GAME__BH) ? BHS__BLACK_HOLE__BITS_PER_COL
                                     : BHS__GOLF__BITS_PER_COL)))
    {
        fprintf(stderr, "Error reading the board at line No. %d!\n",
            error_line_num);
        exit(-1);
    }
    if (unlikely(black_hole_solver_setup(solver)))
    {
        fputs("Could not initialise solver (out-of-memory)\n", stderr);
        exit(-1);
    }

    const int solver_ret_code = solver_run(
        solver, settings.max_iters_limit, settings.iters_display_step);
    FILE *const out_fh = settings_ptr->out_fh;

    if (!solver_ret_code)
    {
        fputs("Solved!\n", out_fh);
    }
    else if (solver_ret_code == BLACK_HOLE_SOLVER__OUT_OF_MEMORY)
    {
        fputs("Out of memory!\n", stderr);
        exit(-1);
    }
    else if (solver_ret_code == BLACK_HOLE_SOLVER__OUT_OF_ITERS)
    {
        fputs("Intractable!\n", out_fh);
    }
    else
    {
        fputs("Unsolved!\n", out_fh);
    }

    fprintf(out_fh, "At most %lu cards could be played.\n",
        black_hole_solver_get_max_num_played_cards(solver));
    fprintf(out_fh,
        "Total number of states checked is %lu.\n"
        "This scan generated %lu states.\n",
        black_hole_solver_get_iterations_num(solver),
        black_hole_solver_get_num_states_in_collection(solver));

    black_hole_solver_recycle(solver);
#undef settings
    return 0;
}

int main(int argc, char *argv[])
{
    int arg_idx;
    bhs_settings settings = parse_cmd_line(argc, argv, &arg_idx);

    for (; arg_idx < argc; ++arg_idx)
    {
        char *const filename = argv[arg_idx];
        fprintf(settings.out_fh, "[= Starting file %s =]\n", filename);
        const int ret = output_stats__solve_file(filename, &settings);
        if (unlikely(ret))
        {
            solve_free(&settings);
            return -1;
        }
        fprintf(settings.out_fh, "[= END of file %s =]\n", filename);
    }
    fflush(settings.out_fh);
    solve_free(&settings);

    return 0;
}
