#include <assert.h>
#include <stdio.h>

#define BUFSIZE 1024

#define COUNTER_TO_PICK(X) ((X) - 'X' + 'A')

int
shape_score(char shape)
{
	assert(shape >= 'A' && shape <= 'C');
	return shape - 'A' + 1;
}

int
outcome_score(char pick, char counter)
{
	assert(pick >= 'A' && pick <= 'C');
	assert(counter >= 'A' && counter <= 'C');
	if (pick == counter) {
		return 3;
	} else if (counter - pick == 1 || (counter == 'A' && pick == 'C')) {
		return 6;
	}

	return 0;
}

char
decide_counter(char strat, char pick)
{
	char counter;

	switch (strat) {
	case 'X':
		counter = pick - 1;
		counter = counter < 'A' ? 'C' : counter;
		return counter;
	case 'Y':
		return pick;
	case 'Z':
		counter = pick + 1;
		counter = counter > 'C' ? 'A' : counter;
		return counter;
	default:
		assert(0);
	}
}

int
main(int argc, char **argv)
{
	FILE *fp;
	char buffer[BUFSIZE];
	char pick, counter, counter2;
	long score, score2;

	if (argc < 2) {
		fprintf(stderr, "usage: %s {FILE}\n", argv[0]);
		return 1;
	}

	fp = fopen(argv[1], "r");
	assert(fp);

	score = score2 = 0;
	while (fgets(buffer, BUFSIZE, fp)) {
		pick = buffer[0];

		counter = buffer[2];
		score += shape_score(COUNTER_TO_PICK(counter));
		score += outcome_score(pick, COUNTER_TO_PICK(counter));

		counter2 = decide_counter(counter, pick);
		score2 += shape_score(counter2);
		score2 += outcome_score(pick, counter2);

	}
	fclose(fp);

	printf("%ld\n", score);
	printf("%ld\n", score2);

	return 0;
}
