#!/bin/bash

mnt_point="/home/shlomif/apps/black-hole-solver/summaries/"
mkdir -p "$mnt_point"

sshfs 'sh:/home/shlomif/progs/games/black-hole-solitaire/trunk/black-hole-solitaire/all-in-a-row-gnu-parallel-range-solving/summaries' "$mnt_point"
