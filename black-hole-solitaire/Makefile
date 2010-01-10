all: run_script

BOARD = 45508856405861261758.bh.board.txt

run_script: $(BOARD)
	perl black-hole-solver.pl $(BOARD)	

$(BOARD):
	./make_pysol_freecell_board.py --pysolfc -t 45508856405861261758 black_hole > $@

