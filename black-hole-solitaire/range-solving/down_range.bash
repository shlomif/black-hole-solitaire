#!/bin/bash
export PATH="$HOME/progs/games/black-hole-solitaire/trunk/black-hole-solitaire/c-solver/build/:$PATH"
perl down-range.pl 2>&1 | tee -a "$$.black-hole.LOG"
