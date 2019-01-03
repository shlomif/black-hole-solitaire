mkdir -p boards
/home/shlomif/progs/freecell/git/fc-solve/fc-solve/source/board_gen/gen-multiple-pysol-layouts --dir boards/ --game golf --prefix golf --suffix .board seq 1 1000
(
  run()
  {
      local args="$1"
      shift
      echo "== ] $fn $args [ =="
      perl -Ilib bin/golf-solitaire-solve-perl $args "$fn"
  }
  for i in $(seq 1 1000)
  do
      fn=boards/golf"$i".board
      echo "== $fn =="
      # run "" || run "--queens-on-kings" || run "--wrap-ranks"
      run "--queens-on-kings"
  done
) | timestamper | tee -a ~/golfs4.txt
