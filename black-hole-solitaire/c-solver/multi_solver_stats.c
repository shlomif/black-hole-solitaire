// Copyright (C) 2018 Shlomi Fish <shlomif@cpan.org>
//
// Distributed under terms of the Expat license.
#include <solver_common.h>
#ifdef BLACK_HOLE_SOLVER_WITH_PYTHON
#include "libpysol_cards/python_embed.h"
#endif
#include <signal.h>

static volatile bool keep_running = true;

static void sigint_handler(int dummy) { keep_running = false; }

static inline int output_stats__solve_board_string(
    const char *const board, bhs_settings *const settings_ptr)
{
#define settings (*settings_ptr)
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
#undef settings
    const int ret = output_stats__solve_board_string(board, settings_ptr);
    return ret;
}

int main(int argc, char *argv[])
{
    int arg_idx;
    bhs_settings settings = parse_cmd_line(argc, argv, &arg_idx);
#ifdef BLACK_HOLE_SOLVER_WITH_PYTHON
    global_python_instance_type global_python_struct;
    global_python_instance_type *const global_python = &global_python_struct;
    global_python_instance__init(global_python);
    pysol_cards__master_instance_type master_instance_struct;
    pysol_cards__master_instance_type *const master_instance =
        &master_instance_struct;
    pysol_cards__master_instance_init(master_instance, global_python);
    pysol_cards__generator_type generator;
    pysol_cards__create_generator(&generator, global_python,
        master_instance->create_gen, settings.game_string, 0);
    signal(SIGINT, sigint_handler);
#endif
    char board_string[BOARD_STRING_SIZE];

    for (; arg_idx < argc; ++arg_idx)
    {
        char *const arg = argv[arg_idx];
        long startidx, endidx;
#ifdef BLACK_HOLE_SOLVER_WITH_PYTHON
        if (!strcmp(arg, "seq"))
        {
            if (arg_idx + 2 + 0 >= argc)
            {
                exit(1);
            }
            startidx = atol(argv[++arg_idx]);
            endidx = atol(argv[++arg_idx]);
            for (long deal_idx = startidx; keep_running && (deal_idx <= endidx);
                ++deal_idx)
            {
                const int ret_code =
                    pysol_cards__deal(&generator, board_string, deal_idx);
                if (ret_code)
                {
                    Py_DECREF(global_python->py_module);
                    fprintf(stderr, "Cannot convert argument\n");
                    return PYSOL_CARDS__FAIL;
                }
                fprintf(
                    settings.out_fh, "[= Starting file deal%ld =]\n", deal_idx);
                output_stats__solve_board_string(board_string, &settings);
#if 0
                if ((deal_idx & ((1 << 12) - 1)) == 0)
                {
                    fprintf(stderr, "Reached %ld\n", deal_idx);
                    fflush(stderr);
                }
#endif
                fprintf(
                    settings.out_fh, "[= END of file deal%ld =]\n", deal_idx);
            }
        }
        else
#endif
        {
            char *const filename = arg;
            fprintf(settings.out_fh, "[= Starting file %s =]\n", filename);
            const int ret = output_stats__solve_file(filename, &settings);
            if (unlikely(ret))
            {
                solve_free(&settings);
                return -1;
            }
            fprintf(settings.out_fh, "[= END of file %s =]\n", filename);
        }
    }
#ifdef BLACK_HOLE_SOLVER_WITH_PYTHON
    pysol_cards__master_instance_release(master_instance);
    global_python_instance__release(global_python);
#endif
    fflush(settings.out_fh);
    solve_free(&settings);

    return 0;
}
