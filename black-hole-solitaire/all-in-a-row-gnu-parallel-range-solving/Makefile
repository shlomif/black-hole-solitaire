BASE = all-in-a-row-PySolFC-first-1M-run-results
ARC_TAR = $(BASE).tar
ARC_TAR_XZ = $(ARC_TAR).xz

all: $(ARC_TAR_XZ)

$(ARC_TAR_XZ): $(ARC_TAR)
	nice -19 xz -9 --extreme < $< > $@

DOCS = $(BASE)/all_in_a_row_SUMMARY-no-prune.txt $(BASE)/all_in_a_row_SUMMARY-prune.txt $(BASE)/README.txt $(BASE)/expand.pl

$(ARC_TAR): $(DOCS)
	tar -cvf $@ $(DOCS)

$(BASE)/all_in_a_row_SUMMARY-no-prune.txt $(BASE)/all_in_a_row_SUMMARY-prune.txt : $(BASE)/%: archived_SUMMARIES/% compress-output.pl Makefile
	mkdir -p $(BASE)
	perl compress-output.pl < $< > $@

$(BASE)/README.txt $(BASE)/expand.pl: $(BASE)/%: docs/%
	mkdir -p $(BASE)
	cp -f $< $@

test: $(ARC_TAR)

