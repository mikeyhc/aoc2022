#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#define BUFSIZE 1024
#define MAXINSTR 256
#define SCREENHEIGHT 6
#define SCREENWIDTH 40
#define SCREENPIXELS (SCREENHEIGHT * SCREENWIDTH)

typedef enum {
	NOOP,
	ADDX
} Op;

typedef struct {
	Op o;
	int v;
} Instr;

void
parse_instr(char *input, Instr *instr)
{
	if (input[0] == 'n') {
		instr->o = NOOP;
	} else if (input[0] == 'a') {
		instr->o = ADDX;
		instr->v = strtol(input + 5, NULL, 10);
		assert(errno != ERANGE);
	} else {
		assert(0);
	}
}

void
inc_cycle(int *cycle, int regx, long *sum)
{
	++*cycle;
	if ((*cycle - 20) % 40 == 0) {
		*sum += regx * *cycle;
	}
	assert(*cycle <= SCREENPIXELS);
}

void
print_screen(char *screen)
{
	int i;

	for (i = 0; i < SCREENPIXELS; ++i) {
		if (screen[i] == 0) {
			putchar('.');
		} else if (screen[i] == 1) {
			putchar('#');
		} else {
			assert(0);
		}

		if ((i + 1) % SCREENWIDTH == 0 && i != 0) {
			putchar('\n');
		}
	}
	putchar('\n');
}

void
draw_pixel(int cycle, int regx, char *screen)
{
	if ((cycle % 40) >= regx-1 && (cycle % 40) <= regx+1) {
		screen[cycle] = 1;
	}
}

int
main(int argc, char **argv)
{
	FILE *fp;
	char buffer[BUFSIZE], screen[SCREENPIXELS];
	Instr instrs[MAXINSTR];
	int instr_count, regx, cycle;
	int i;
	long sum = 0;

	if (argc < 2) {
		fprintf(stderr, "usage: %s {FILE}\n", argv[0]);
		return 1;
	}

	fp = fopen(argv[1], "r");
	assert(fp);

	for (i = 0; i < SCREENPIXELS; ++i) {
		screen[i] = 0;
	}

	instr_count = 0;
	while(fgets(buffer, BUFSIZE, fp)) {
		parse_instr(buffer, instrs + instr_count);
		++instr_count;
	}
	fclose(fp);

	regx = 1;
	cycle = 0;
	for (i = 0; i < instr_count; ++i) {
		switch (instrs[i].o) {
		case NOOP:
			draw_pixel(cycle, regx, screen);
			inc_cycle(&cycle, regx, &sum);
			break;
		case ADDX:
			draw_pixel(cycle, regx, screen);
			inc_cycle(&cycle, regx, &sum);
			draw_pixel(cycle, regx, screen);
			inc_cycle(&cycle, regx, &sum);
			regx += instrs[i].v;
		}
	}

	printf("%ld\n", sum);
	print_screen(screen);

	return 0;
}
