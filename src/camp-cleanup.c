#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#define BUFSIZE 1024

typedef struct {
	int start, end;
} Range;

void
parse_pair(char *input, Range *a, Range *b)
{
	char *end;

	a->start = strtol(input, &end, 10);
	assert(errno != ERANGE);
	assert(*end == '-');

	a->end = strtol(end + 1, &end, 10);
	assert(errno != ERANGE);
	assert(*end == ',');

	b->start = strtol(end + 1, &end, 10);
	assert(errno != ERANGE);
	assert(*end == '-');

	b->end = strtol(end + 1, &end, 10);
	assert(errno != ERANGE);
}

int
range_in_range(Range a, Range b)
{
	return a.start <= b.start && a.end >= b.end;
}

int
point_in_range(Range a, int p)
{
	return p >= a.start && p <= a.end;
}

int
main(int argc, char **argv)
{
	FILE *fp;
	char buffer[BUFSIZE];
	Range a, b;
	long count, count2;

	if (argc < 2) {
		fprintf(stderr, "usage: %s {FILE}\n", argv[0]);
		return 1;
	}

	fp = fopen(argv[1], "r");
	assert(fp);

	count = count2 = 0;
	while (fgets(buffer, BUFSIZE, fp)) {
		parse_pair(buffer, &a, &b);

		if (range_in_range(a, b) || range_in_range(b, a)) {
			++count;
		}

		if (point_in_range(a, b.start) || point_in_range(a, b.end)) {
			++count2;
		} else if (point_in_range(b, a.start) ||
				point_in_range(b, a.end)) {
			++count2;
		}
	}
	fclose(fp);

	printf("%ld\n", count);
	printf("%ld\n", count2);

	return 0;
}
