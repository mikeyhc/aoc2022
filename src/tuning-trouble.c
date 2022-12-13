#include <assert.h>
#include <stdio.h>
#include <string.h>

#define BUFSIZE 8192
#define MAX_CHARS ('z' - 'a' + 1)

int
main(int argc, char **argv)
{
	FILE *fp;
	char buffer[BUFSIZE], *start, *msg;
	char seen[MAX_CHARS];
	int i = 0;

	if (argc < 2) {
		fprintf(stderr, "usage: %s {FILE}\n", argv[0]);
		return 1;
	}

	fp = fopen(argv[1], "r");
	assert(fp);
	assert(fgets(buffer, BUFSIZE, fp) != 0);
	fclose(fp);

	for (start = buffer; start[4] != '\0' && start[4] != '\n'; ++start) {
		memset(seen, 0, sizeof(seen));
		for (i = 0; i < 4; ++i) {
			seen[*(start + i) - 'a'] += 1;
		}

		for (i = 0; i < MAX_CHARS; ++i) {
			if (seen[i] > 1) {
				break;
			}
		}

		if (i == MAX_CHARS) {
			break;
		}
	}

	for (msg = start; msg[14] != '\0' && msg[14] != '\n'; ++msg) {
		memset(seen, 0, sizeof(seen));
		for (i = 0; i < 14; ++i) {
			seen[*(msg + i) - 'a'] += 1;
		}

		for (i = 0; i < MAX_CHARS; ++i) {
			if (seen[i] > 1) {
				break;
			}
		}

		if (i == MAX_CHARS) {
			break;
		}
	}


	printf("%ld\n", start - buffer + 4);
	printf("%ld\n", msg - buffer + 14);

	return 0;
}
