#! /usr/bin/env python3
# -*- coding: utf-8 -*-
# vim:fenc=utf-8
#
# Copyright © 2020 Shlomi Fish < https://www.shlomifish.org/ >
#
# Licensed under the terms of the MIT license.

import re
import sys

should_process = None

if sys.argv[1] == '--process=no_max_num_played':
    should_process = True
elif sys.argv[1] == '--process=none':
    should_process = False
else:
    raise Exception("wrong invocation")


def _newlinify(text, m):
    start = text.rindex("\n", 0, m.start(0)+1)
    end = text.index("\n", m.end(0)-1, -1)
    return (text[0:start] + "".join([
        x for x in text[start:end] if x == '\n']), text[end:])


def _remove(text, pat):
    m = re.search(pat, text, flags=(re.M | re.S))
    assert m
    return _newlinify(text, m)


def process_black_hole_solver_h(text):
    """docstring for process_black_hole_solver_h"""
    pre, suf = _remove(
        text,
        "^// Added.*?DLLEXPORT extern unsigned " +
        "long black_hole_solver_get_max_num_played_cards\\(.*?(?:\\n){2,2}"
    )
    return pre + suf


def _clear_all_individual_lines(text, pat):
    """docstring for _clear_all_individual_lines"""
    while True:
        m = re.search(pat, text)
        if m:
            text1, text2 = _newlinify(text, m)
            text = text1 + text2
        else:
            break
    return text


def process_lib_c(text):
    """docstring for process_lib_c"""
    pre, text = _remove(
        text, "^ *var_AUTO\\(\\s*max_reached_depths_stack_len.*?\\);$")
    pre2, text = _remove(
        text, "^ *while \\(current_depths_stack_len.*?^ *\\}$")
    pre3, text = _remove(
        text, "^( *)if \\(was_moved\\).*?^\\1\\}$")
    pre4, text = _remove(
        text, "^DLLEXPORT extern[^\n]+\n" +
        "black_hole_solver_get_max_num_played_cards.*?^\\}")
    text = pre + pre2 + pre3 + pre4 + text
    return _clear_all_individual_lines(
        text, "(?:depths_stack|max_num_played|prev_len|was_moved)"
    )


def process_solver_common_h(text):
    """docstring for process_black_hole_solver_h"""
    pre, text = _remove(
        text, "^ *else if \\(unlikely\\(" +
        "\n[^\\n]*\"--show-max-num-played-cards\".*?^ *\\}$")
    pre2, text = _remove(
        text, "^ *if \\(unlikely\\(settings\\." +
        "show_max_num_played_cards.*?^ *\\}$")
    text = pre + pre2 + text
    return _clear_all_individual_lines(text, "max_num_played")


def wrapper(basename, cb):
    global should_process
    from pathlib import Path

    src_fn = Path(sys.argv[0]).parent.parent / basename
    dest_fn = Path(".") / "generated" / basename

    p = dest_fn.parent
    if not p.exists():
        p.mkdir(parents=True)
    with open(dest_fn, "wt") as ofh:
        with open(src_fn, "rt") as ifh:
            text = ifh.read()
            ofh.write(cb(text) if should_process else text)


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
