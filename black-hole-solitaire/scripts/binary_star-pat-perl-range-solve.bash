mkdir -p boards
~/progs/freecell/git/fc-solve/fc-solve/source/board_gen/gen-multiple-pysol-layouts --dir boards/ --game binary_star --prefix binary_star --suffix .board seq 1 10000
(
  perl_cmd_line="perl -Ilib bin/black-hole-solve --max-iters 240000 --num-foundations 2 --display-boards"
  c_cmd_line="./black-hole-solve --game binary_star --display-boards"
  cmd="$c_cmd_line"
  cmd="$perl_cmd_line"
  run()
  {
      local args="$1"
      shift
      echo "== ] $fn $args [ =="
      $cmd $args "$fn"
  }
  mkdir -p "solutions"
  i=1
  while test "$i" -le "10000"
  do
      fn=boards/binary_star"$i".board
      outfn=solutions/binary_star"$i".sol
      echo "== $fn =="
      run "" | tee "$outfn"
      # run "--queens-on-kings"
      let ++i
  done
) |& tee ~/solver-log-binary_star1.txt | timestamper-with-elapsed
