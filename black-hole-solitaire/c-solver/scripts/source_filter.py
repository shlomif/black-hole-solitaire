#! /usr/bin/env python3
# -*- coding: utf-8 -*-
# vim:fenc=utf-8
#
# Copyright © 2020 Shlomi Fish < https://www.shlomifish.org/ >
#
# Licensed under the terms of the MIT license.
"""
source_filter for removing the max_num_played feature from
black-hole-solver's C code.
"""

import re
import sys
from pathlib import Path

if sys.argv[1] == '--process=no_max_num_played':
    SHOULD_PROCESS = True
elif sys.argv[1] == '--process=none':
    SHOULD_PROCESS = False
else:
    raise Exception("wrong invocation")


def _newlinify(text, match_object):
    start = text.rindex("\n", 0, match_object.start(0)+1)
    end = text.index("\n", match_object.end(0)-1, -1)
    prefix = text[0:start] + "".join([
        x for x in text[start:end] if x == '\n'
    ])
    return (prefix, text[end:])


def _remove(text, pat):
    match_object = re.search(pat, text, flags=(re.M | re.S))
    assert match_object
    return _newlinify(text, match_object)


def _multi_remove(text, patterns_list):
    if len(patterns_list) == 0:
        return text
    prefix, remaining_text = _remove(text, patterns_list[0])
    return prefix + _multi_remove(remaining_text, patterns_list[1:])


def process_black_hole_solver_h(text):
    """docstring for process_black_hole_solver_h"""
    return _multi_remove(
        text,
        [
            "^// Added.*?DLLEXPORT extern unsigned " +
            "long black_hole_solver_get_max_num_played_cards" +
            "\\(.*?(?:\\n){2,2}",
        ]
    )


def _clear_all_individual_lines(text, pat):
    """docstring for _clear_all_individual_lines"""
    return re.sub(
        '^[^\\n]*?(?:'+pat+')[^\\n]*?$', '', text, flags=(re.M | re.S)
    )


def process_lib_c(text):
    """docstring for process_lib_c"""
    out_text = _multi_remove(
        text,
        [
         "^ *var_AUTO\\(\\s*max_reached_depths_stack_len.*?\\);$",
         "^ *while \\(current_depths_stack_len.*?^ *\\}$",
         "^( *)if \\(was_moved\\).*?^\\1\\}$",
         ("^DLLEXPORT extern[^\n]+\n" +
             "black_hole_solver_get_max_num_played_cards.*?^\\}"),
        ]
    )
    return _clear_all_individual_lines(
        out_text, "(?:depths_stack|max_num_played|prev_len|was_moved)"
    )


def process_solver_common_h(text):
    """docstring for process_black_hole_solver_h"""
    out_text = _multi_remove(
        text,
        [
         ("^ *else if \\(unlikely\\(" +
          "\n[^\\n]*\"--show-max-num-played-cards\".*?^ *\\}$"),
         ("^ *if \\(unlikely\\(settings\\." +
          "show_max_num_played_cards.*?^ *\\}$"),
        ]
    )
    return _clear_all_individual_lines(out_text, "max_num_played")


def wrapper(basename, callback):
    """optionally filter file basename through callback"""
    src_fn = Path(sys.argv[0]).parent.parent / basename
    dest_fn = Path(".") / "generated" / basename

    dest_parent = dest_fn.parent
    if not dest_parent.exists():
        dest_parent.mkdir(parents=True)
    with open(dest_fn, "wt") as ofh:
        with open(src_fn, "rt") as ifh:
            text = ifh.read()
            ofh.write(callback(text) if SHOULD_PROCESS else text)


wrapper(
    "include/black-hole-solver/black_hole_solver.h",
    process_black_hole_solver_h
)

wrapper(
    "solver_common.h",
    process_solver_common_h
)

wrapper(
    "lib.c",
    process_lib_c
)
