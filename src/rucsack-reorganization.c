#include <assert.h>
#include <stdio.h>
#include <string.h>

#define BUFSIZE 10244
#define VALUE_RANGE 52 /* a-z + A-Z */

typedef struct {
	int first, second;
} Tracker;

int
value_priority(char value) {
	if (value >= 'a' && value <= 'z') {
		return value - 'a' + 1;
	} else if (value >= 'A' && value <= 'Z') {
		return value - 'A' + 27;
	}

	assert(0);
}

int
main(int argc, char **argv)
{
	FILE *fp;
	char buffer[BUFSIZE];
	Tracker seen[VALUE_RANGE];
	char badge[VALUE_RANGE][3];
	size_t size, i;
	long sum, badge_sum;
	int pos;

	if (argc < 2) {
		fprintf(stderr, "usage: %s {FILE}\n", argv[0]);
		return 1;
	}

	fp = fopen(argv[1], "r");
	assert(fp);

	for (i = 0; i < VALUE_RANGE; ++i) {
		badge[i][0] = badge[i][1] = badge[i][2] = 0;
	}

	sum = badge_sum = 0;
	pos = 0;
	while (fgets(buffer, BUFSIZE, fp)) {
		size = strlen(buffer) / 2;

		for (i = 0; i < VALUE_RANGE; ++i) {
			seen[i].first = seen[i].second = 0;
		}

		for (i = 0; i < size; ++i) {
			seen[value_priority(buffer[i]) - 1].first = 1;
			badge[value_priority(buffer[i]) - 1][pos] = 1;
		}

		for (i = size; i < size * 2; ++i) {
			seen[value_priority(buffer[i]) - 1].second = 1;
			badge[value_priority(buffer[i]) - 1][pos] = 1;
		}

		for (i = 0; i < VALUE_RANGE; ++i) {
			if (seen[i].first && seen[i].second) {
				sum += i + 1;
				break;
			}
		}

		if (pos == 2) {
			for (i = 0; i < VALUE_RANGE; ++i) {
				if (badge[i][0] && badge[i][1] && badge[i][2]) {
					badge_sum += i + 1;
					break;
				}
			}

			for (i = 0; i < VALUE_RANGE; ++i) {
				badge[i][0] = badge[i][1] = badge[i][2] = 0;
			}

			pos = 0;
		} else {
			++pos;
		}
	}

	fclose(fp);

	printf("%ld\n", sum);
	printf("%ld\n", badge_sum);

	return 0;
}
