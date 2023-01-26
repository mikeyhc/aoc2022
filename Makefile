APPS = calorie-counting rock-paper-scissors rucsack-reorganization \
       camp-cleanup supply-stacks tuning-trouble no-space-left \
       treetop-tree-house rope-bridge cathode-ray-tube monkey-middle \
       hill-climbing
OUTDIR = bin
OUTPUT = $(APPS:%=$(OUTDIR)/%)

all: $(OUTDIR) $(OUTPUT)

$(OUTDIR)/hill-climbing: LDFLAGS += -lm

$(OUTDIR)/%: src/%.c
	$(CC) -o $@ -Wall -Wextra -pedantic $(LDFLAGS) -g $^

$(OUTDIR):
	mkdir $(OUTDIR)

clean:
	rm -r $(OUTDIR)

.PHONY: all clean
