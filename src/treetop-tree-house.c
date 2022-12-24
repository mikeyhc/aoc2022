#include <assert.h>
#include <stdio.h>

#define BUFSIZE 1024
#define MAXCOLS 128
#define MAXROWS 128

void
map_size(char map[MAXROWS][MAXCOLS], long *height, long *width)
{
	for (*height = 0; map[*height][0] != -1; ++*height) ;
	for (*width = 0; map[0][*width] != -1; ++*width) ;
}

long
count_seen(char map[MAXROWS][MAXCOLS], char seen[MAXROWS][MAXCOLS])
{
	int row, col, highest;
	long height, width, total = 0;

	map_size(map, &height, &width);

	for (row = 0; row < height; ++row) {
		/* scan rows */
		highest = -1;
		for (col = 0; col < width; ++col) {
			if (map[row][col] > highest) {
				highest = map[row][col];
				if (!seen[row][col]) {
					++total;
					seen[row][col] = 1;
				}
			}

			if (map[row][col] == 9) {
				break;
			}
		}


		/* reverse scan rows */
		highest = -1;
		for (col = width - 1; col >= 0; --col) {
			if (map[row][col] > highest) {
				highest = map[row][col];
				if (!seen[row][col]) {
					++total;
					seen[row][col] = 1;
				}
			}

			if (map[row][col] == 9) {
				break;
			}
		}
	}

	for (col = 0; col < width; ++col) {
		/* scan cols */
		highest = -1;
		for (row = 0; row < height; ++row) {
			if (map[row][col] > highest) {
				highest = map[row][col];
				if (!seen[row][col]) {
					++total;
					seen[row][col] = 1;
				}
			}

			if (map[row][col] == 9) {
				break;
			}
		}

		/* scan cols */
		highest = -1;
		for (row = height - 1; row >= 0; --row) {
			if (map[row][col] > highest) {
				highest = map[row][col];
				if (!seen[row][col]) {
					++total;
					seen[row][col] = 1;
				}
			}

			if (map[row][col] == 9) {
				break;
			}
		}
	}

	/*
	for (row = 0; row < height; ++row) {
		for (col = 0; col < width; ++col) {
			if (seen[row][col]) {
				putchar('X');
			} else {
				putchar(' ');
			}
		}
		putchar('\n');
	}
	*/


	return total;
}

long
score_point(char map[MAXROWS][MAXCOLS], int y, int x, long height, long width)
{
	long score;
	int i, cur;

	cur = map[y][x];
	for (i = 1; x - i > 0; ++i) {
		if (cur <= map[y][x - i])
			break;
	}
	score = i;

	for (i = 1; y - i > 0; ++i) {
		if (cur <=  map[y - i][x]) {
			break;
		}
	}
	score *= i;

	for (i = 1; x + i < width - 1; ++i) {
		if (cur <= map[y][x + i])
			break;
	}
	score *= i;

	for (i = 1; y + i < height - 1; ++i) {
		if (cur <=  map[y + i][x]) {
			break;
		}
	}
	score *= i;

	return score;
}

long
scenic_score(char map[MAXROWS][MAXCOLS])
{
	int row, col;
	long height, width, tmp, max_score = 0;

	map_size(map, &height, &width);

	for (row = 1; row < height - 1; ++row) {
		for (col = 1; col < width - 1; ++col) {
			tmp = score_point(map, row, col, height, width);
			max_score = tmp > max_score ? tmp : max_score;
		}
	}

	return max_score;
}

int
main(int argc, char **argv)
{
	FILE *fp;
	char buffer[BUFSIZE];
	char map[MAXROWS][MAXCOLS], seen[MAXROWS][MAXCOLS];
	int i, j;

	if (argc < 2) {
		fprintf(stderr, "usage: %s {FILE}\n", argv[0]);
		return 1;
	}

	for (i = 0; i < MAXROWS; ++i) {
		for (j = 0; j < MAXCOLS; ++j) {
			map[i][j] = -1;
			seen[i][j] = 0;
		}
	}

	fp = fopen(argv[1], "r");
	assert(fp);

	j = 0;
	while(fgets(buffer, BUFSIZE, fp)) {
		for (i = 0; buffer[i] != '\n' && buffer[i] != '\0'; ++i) {
			map[j][i] = buffer[i] - '0';
		}
		++j;
	}
	fclose(fp);

	printf("%ld\n", count_seen(map, seen));
	printf("%ld\n", scenic_score(map));

	return 0;
}
