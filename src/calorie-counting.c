#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#define BUFSIZE 1024

int
main(int argc, char **argv)
{
	FILE *fp;
	char buffer[BUFSIZE], *end;
	long sum, max;
	long max1, max2, max3;

	if (argc < 2) {
		fprintf(stderr, "usage: %s {FILE}\n", argv[0]);
		return 1;
	}

	fp = fopen(argv[1], "r");
	assert(fp);
	sum = max = 0;
	max1 = max2 = max3 = 0;
	while (fgets(buffer, BUFSIZE, fp)) {
		if (buffer[0] == '\n') {
			max = sum > max ? sum : max;
			if (sum > max1) {
				max3 = max2;
				max2 = max1;
				max1 = sum;
			} else if (sum > max2) {
				max3 = max2;
				max2 = sum;
			} else if (sum > max3) {
				max3 = sum;
			}

			sum = 0;
			continue;
		}

		sum += strtol(buffer, &end, 10);
		assert(errno != ERANGE);
	}
	fclose(fp);

	max = sum > max ? sum : max;
	if (sum > max1) {
		max3 = max2;
		max2 = max1;
		max1 = sum;
	} else if (sum > max2) {
		max3 = max2;
		max2 = sum;
	} else if (sum > max3) {
		max3 = sum;
	}

	printf("%ld\n", max);
	printf("%ld\n", max1 + max2 + max3);

	return 0;
}
