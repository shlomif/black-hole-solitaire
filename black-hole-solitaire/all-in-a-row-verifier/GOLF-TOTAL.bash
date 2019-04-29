solve()
{
    local deal_idx="$1"
    shift

    echo "Solving deal $deal_idx"

    local deal_fn="$deal_idx.golf.board"
    local sol_fn="$deal_idx.golf.sol"
    make_pysol_freecell_board.py -F -t "$deal_idx" golf > "$deal_fn"
    if black-hole-solve --game golf "$deal_fn" > "$sol_fn"
    then
        if ! perl verify_golf.pl "$deal_fn" "$sol_fn"
        then
            echo "Error in solution in deal No. $deal_idx"
            return -1
        fi
    else
        echo "Unsolved";
    fi
    return 0
}
