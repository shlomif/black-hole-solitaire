#!/bin/bash

source 'prepare.bash'

# Configuration
start_idx="1"
end_idx="1,000,000"
num_cpus="4"

if test -f SUMMARY.txt ; then
    start_idx="$(tail -1 SUMMARY.txt | perl -lpe 's/\D.*//')"
    let start_idx++
fi

seq "${start_idx//,/}" "${end_idx//,/}" | parallel -j"$num_cpus" perl summarize.pl {}
