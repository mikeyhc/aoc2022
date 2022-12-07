APPS = calorie-counting rock-paper-scissors rucsack-reorganization camp-cleanup
OUTDIR = bin
OUTPUT = $(APPS:%=$(OUTDIR)/%)

all: $(OUTDIR) $(OUTPUT)

$(OUTDIR)/%: src/%.c
	$(CC) -o $@ -Wall -Wextra -pedantic -g $^

$(OUTDIR):
	mkdir $(OUTDIR)

clean:
	rm -r $(OUTDIR)

.PHONY: all clean
