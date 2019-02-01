function solve()
{
    deal_idx="$1"
    shift

    echo "Solving deal $deal_idx"

    deal_fn="$deal_idx.all_in_a_row.board"
    sol_fn="$deal_idx.all_in_a_row.sol"
    make_pysol_freecell_board.py -t "$deal_idx" all_in_a_row > "$deal_fn"
    if black-hole-solve --game all_in_a_row "$deal_fn" > "$sol_fn" ; then
        if ! perl verify_all_in_a_row.pl "$deal_fn" "$sol_fn" ; then
            echo "Error in solution in deal No. $deal_idx"
            exit -1
        fi
    else
        echo "Unsolved";
    fi
}
