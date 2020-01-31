mkdir -p boards
~/progs/freecell/git/fc-solve/fc-solve/source/board_gen/gen-multiple-pysol-layouts --dir boards/ --game golf --prefix golf --suffix .board seq 1 10000
(
  perl_cmd_line="perl -Ilib bin/golf-solitaire-solve-perl"
  c_cmd_line="./black-hole-solve --game golf --display-boards"
  cmd="$c_cmd_line"
  run()
  {
      local args="$1"
      shift
      echo "== ] $fn $args [ =="
      $cmd $args "$fn"
  }
  for i in $(seq 1 10000)
  do
      fn=boards/golf"$i".board
      echo "== $fn =="
      run "" || run "--queens-on-kings" || run "--wrap-ranks"
      # run "--queens-on-kings"
  done
) | timestamper | tee -a ~/golfs6.txt
