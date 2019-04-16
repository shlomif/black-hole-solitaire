mkdir -p boards
/home/shlomif/progs/freecell/git/fc-solve/fc-solve/source/board_gen/gen-multiple-pysol-layouts --dir boards/ --game golf --prefix golf --suffix .board seq 1 10000
(
  perl_cmd_line="perl -Ilib bin/golf-solitaire-solve-perl"
  c_cmd_line="./black-hole-solve --game golf"
  cmd="$c_cmd_line"
  run()
  {
      local args="$1"
      shift
      echo "[= Starting file $fn =]"
      $cmd $args "$fn"
      echo "[= END of file $fn =]"
  }
  for i in $(seq 1 200)
  do
      fn=boards/golf"$i".board
      run "" # || run "--queens-on-kings" || run "--wrap-ranks"
      # run "--queens-on-kings"
  done
)
