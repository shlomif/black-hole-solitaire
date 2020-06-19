# -*- coding: utf-8 -*-
# vim:fenc=utf-8
#
# Copyright © 2019 Shlomi Fish <shlomif@cpan.org>
#
# Distributed under the terms of the Expat license.

import platform

from cffi import FFI


class BlackHoleSolver(object):
    # TEST:$num_befs_weights=5;
    NUM_BEFS_WEIGHTS = 5
    FCS_STATE_SUSPEND_PROCESS = 5
    BHS__BLACK_HOLE__BITS_PER_COL = 2
    BHS__BLACK_HOLE__MAX_NUM_CARDS_IN_COL = 3
    BHS__BLACK_HOLE__NUM_COLUMNS = 17

    def __init__(self, ffi=None, lib=None):
        if ffi:
            self.ffi = ffi
            self.lib = lib
        else:
            self.ffi = FFI()
            self.lib = self.ffi.dlopen(
                "libblack_hole_solver." + (
                    "dll" if (platform.system() == 'Windows') else "so"))
            self.ffi.cdef('''
typedef struct
{
    unsigned long nothing;
} black_hole_solver_instance_t;
int black_hole_solver_create(
black_hole_solver_instance_t **ret_instance);


int black_hole_solver_read_board(
black_hole_solver_instance_t *instance, const char *board_string,
int *error_line_number, unsigned int num_columns,
unsigned int max_num_cards_in_col, unsigned int bits_per_column);

int black_hole_solver_set_max_iters_limit(
black_hole_solver_instance_t *instance, unsigned long limit);

int black_hole_solver_enable_place_queens_on_kings(
black_hole_solver_instance_t *instance, bool enabled_status);

int black_hole_solver_enable_wrap_ranks(
black_hole_solver_instance_t *instance, bool enabled_status);

int black_hole_solver_enable_rank_reachability_prune(
black_hole_solver_instance_t *instance, bool enabled_status);

#define BLACK_HOLE_SOLVER__API__REQUIRES_SETUP_CALL 1
int black_hole_solver_config_setup(
black_hole_solver_instance_t *instance);
int black_hole_solver_setup(
black_hole_solver_instance_t *instance);

int black_hole_solver_run(
black_hole_solver_instance_t *instance);

int black_hole_solver_recycle(
black_hole_solver_instance_t *instance);

int black_hole_solver_free(
black_hole_solver_instance_t *instance);

void black_hole_solver_init_solution_moves(
black_hole_solver_instance_t *instance);
int black_hole_solver_get_next_move(
black_hole_solver_instance_t *instance, int *col_idx_ptr,
int *card_rank_ptr, int *card_suit_ptr /* Will return H=0, C=1, D=2, S=3 */
);

unsigned long black_hole_solver_get_num_states_in_collection(
black_hole_solver_instance_t *instance);

unsigned long black_hole_solver_get_iterations_num(
black_hole_solver_instance_t *instance);

int black_hole_solver_get_current_solution_board(
black_hole_solver_instance_t *instance, char *output);

const char *black_hole_solver_get_lib_version(void);
''')
        self.user_container = self.ffi.new('black_hole_solver_instance_t * *')
        self.error_on_line = self.ffi.new('int *')
        assert 0 == self.lib.black_hole_solver_create(self.user_container)
        self.user = self.user_container[0]
        self.lib.black_hole_solver_enable_rank_reachability_prune(
            self.user, 1)
        self.lib.black_hole_solver_enable_wrap_ranks(self.user, 1)
        self.lib.black_hole_solver_enable_place_queens_on_kings(
            self.user, 1)
        self.lib.black_hole_solver_config_setup(self.user)

    def new_bhs_user_handle(self):
        return self.__class__(ffi=self.ffi, lib=self.lib)

    def ret_code_is_suspend(self, ret_code):
        """docstring for ret_code_is_suspend"""
        return ret_code == self.FCS_STATE_SUSPEND_PROCESS

    def get_next_move(self):
        move = self.ffi.new('fcs_move_t *')
        num_moves = self.lib.freecell_solver_user_get_moves_left(self.user)
        if not num_moves:
            return None
        ret = self.lib.freecell_solver_user_get_next_move(self.user, move)
        success = 0
        return (move if ret == success else None)

    def input_cmd_line(self, cmd_line_args):
        return {'last_arg': 0,
                'cmd_line_args_len': len(cmd_line_args)}

    def __del__(self):
        self.lib.black_hole_solver_free(self.user)

    def read_board(self, board):
        ret = self.lib.black_hole_solver_read_board(
            self.user,
            bytes(board, 'UTF-8'),
            self.error_on_line,
            self.BHS__BLACK_HOLE__NUM_COLUMNS,
            self.BHS__BLACK_HOLE__MAX_NUM_CARDS_IN_COL,
            self.BHS__BLACK_HOLE__BITS_PER_COL,
        )
        assert ret == 0
        assert 0 == self.lib.black_hole_solver_setup(self.user)
        return ret

    def resume_solution(self):
        return self.lib.black_hole_solver_run(self.user)

    def limit_iterations(self, max_iters):
        self.lib.black_hole_solver_set_max_iters_limit(
            self.user,
            max_iters
        )

    def get_num_times(self):
        return self.lib.freecell_solver_user_get_num_times_long(
            self.user)

    def get_num_states_in_collection(self):
        return self.lib.freecell_solver_user_get_num_states_in_collection_long(
            self.user)

    def recycle(self):
        self.lib.freecell_solver_user_recycle(self.user)
