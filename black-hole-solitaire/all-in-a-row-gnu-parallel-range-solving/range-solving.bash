#!/bin/bash

source 'prepare.bash'

# Configuration
start_idx="1"
end_idx="1,000,000"
# end_idx="10,000"
# end_idx="1,000"
num_cpus="4"
game="black_hole"
prune="true"

if test -f SUMMARY.txt ; then
    start_idx="$(tail -1 SUMMARY.txt | perl -lpe 's/\D.*//')"
    let start_idx++
fi

args=''
args+=" --game=$game"
if $prune ; then
    args+=" --prune"
fi

# --sshlogin 4/sh --sshlogin 2/lap \
# -j4 \


seq "${start_idx//,/}" "${end_idx//,/}" | \
    parallel --ungroup \
    -j4 \
    --max-args 16 \
    perl "$(pwd)"/summarize.pl $args {}
