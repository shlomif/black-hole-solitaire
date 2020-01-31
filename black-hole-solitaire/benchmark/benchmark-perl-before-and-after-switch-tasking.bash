#!/usr/bin/env bash

# set -e -x
out_dir='../foo'
if ! test -d "$out_dir"
then
    mkdir -p "$out_dir"
    gen-multiple-pysol-layouts --dir "$out_dir" --ms --prefix '' --suffix .board --game black_hole seq 1 32000
fi

max_idx="100"

log()
{
    local log_fn="$1"
    shift
    cat -n | timestamper | tee -a "$log_fn"
}

new_feature_args=(--seed 0 --task-name s0 --next-task --seed 1 --task-name s1 --next-task --seed 2 --task-name s2 --next-task --seed 3 --task-name s3 --prelude '200@s2,300@s1,400@s1,500@s0')
old_ver_args=()

run()
{
    local log_fn="$1"
    shift
    declare -a my__run_args=("${!1}")
    shift
    (
        local i=1
        while test "$i" -le "$max_idx"
        do
            perl -Ilib bin/black-hole-solve "${my__run_args[@]}" --quiet "$out_dir/$i.board"
            (( ++i ))
        done
    ) | log "$log_fn"
}

if test -z "$SKIP"
then
    run "new.log.txt" new_feature_args[@]
    run "old.log.txt" old_ver_args[@]
fi

f()
{
    export TITLE="$2"
    cat "$1" | head -"$max_idx" | perl -lanE 'say +($F[0]-($x//=$F[0]))||$ENV{TITLE}'
}
paste -d ,, <(seq 1 "$max_idx") <(f old.log.txt Old) <(f new.log.txt New) | csv2chart xlsx -o foo.xlsx --exec gnumeric
