#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#define BUFSIZE 1024
#define MAPWIDTH 8192
#define MAPHEIGHT 8192
#define MAPSIZE MAPWIDTH * MAPHEIGHT
#define MAXMOVES 2048
#define TOTALKNOTS 10

#define MAPPOS(X, Y) \
	(((Y + (MAPHEIGHT / 2)) * MAPWIDTH) + \
	 (X + (MAPWIDTH / 2)))

typedef enum {
	UP,
	LEFT,
	DOWN,
	RIGHT
} Direction;

typedef struct {
	Direction dir;
	int mag;
} Move;

typedef struct {
	int x, y;
} Point;

void
parse_move(char *input, Move *move)
{
	switch (*input) {
	case 'U':
		move->dir = UP;
		break;
	case 'L':
		move->dir = LEFT;
		break;
	case 'D':
		move->dir = DOWN;
		break;
	case 'R':
		move->dir = RIGHT;
		break;
	default:
		assert(0);
	}

	move->mag = strtol(input + 2, NULL, 10);
	assert(errno != ERANGE);
}

int
apply_move(Move *move, Point *point)
{
	static Direction dir;
	static int mag;
	/* this is dumb but fun */
	if (move != NULL) {
		dir = move->dir;
		mag = move->mag;
	}

	if (mag <= 0) {
		return 0;
	}

	switch (dir) {
	case UP:
		--point->y;
		break;
	case LEFT:
		--point->x;
		break;
	case DOWN:
		++point->y;
		break;
	case RIGHT:
		++point->x;
		break;
	default:
		assert(0);
	}

	--mag;

	return 1;
}

int
adjacent(Point *a, Point *b)
{
	return abs(a->x - b->x) <= 1 && abs(a->y - b->y) <= 1;
}

void
adjust_tail(char *map, Point *head, Point *tail)
{
	while (!adjacent(head, tail)) {
		if (tail->x != head->x) {
			tail->x += head->x < tail->x ? -1 : 1;
		}

		if (tail->y != head->y) {
			tail->y += head->y < tail->y ? -1 : 1;
		}

		if (map != NULL) {
			map[MAPPOS(tail->x, tail->y)] = 1;
		}
	}
}

char
dir_to_char(Direction d)
{
	switch (d) {
	case UP: return 'U';
	case DOWN: return 'D';
	case LEFT: return 'L';
	case RIGHT: return 'R';
	default: assert(0);
	}
}

void
print_map(Move *move, Point *knots)
{
	int i, j;
	int map[41][41];

	printf("\n== %c %d ==\n\n", dir_to_char(move->dir), move->mag);

	for (i = 0; i < 41; ++i) {
		for (j = 0; j < 41; ++j) {
			map[i][j] = 0;
		}
	}

	for (i = 0; i < TOTALKNOTS; ++i) {
		if (i == 0) {
			map[knots[i].y+20][knots[i].x+20] = 'H';
		} else if (map[knots[i].y+20][knots[i].x+20] == 0) {
			map[knots[i].y+20][knots[i].x+20] = i + '0';
		}
	}

	for (i = 0; i < 41; ++i) {
		for (j = 0; j < 41; ++j) {
			if (map[i][j]) {
				putchar(map[i][j]);
			} else if (i == 20 && j == 20) {
				putchar('s');
			} else {
				putchar('.');
			}
		}

		putchar('\n');
	}
}

void
print_visited(char *map)
{
	int i, j;

	putchar('\n');
	for (i = -20; i <= 20; ++i) {
		for (j = -20; j <= 20; ++j) {
			if (i == 0 && j == 0) {
				putchar('s');
			} else if (map[MAPPOS(j,i)]) {
				putchar('#');
			} else {
				putchar('.');
			}
		}
		putchar('\n');
	}
	putchar('\n');
}

int
main(int argc, char **argv)
{
	FILE *fp;
	char buffer[BUFSIZE];
	char *map, *map2;
	Move moves[MAXMOVES];
	Point knots[TOTALKNOTS];
	int total_moves, i, j;
	long score, score2;



	if (argc < 2) {
		fprintf(stderr, "usage: %s {FILE}\n", argv[0]);
		return 1;
	}

	fp = fopen(argv[1], "r");
	assert(fp);

	map = malloc(MAPSIZE);
	assert(map);
	map2 = malloc(MAPSIZE);
	assert(map2);

	for (i = 0; i < MAPSIZE; ++i) {
		map[i] = 0;
		map2[i] = 0;
	}
	map[MAPPOS(0, 0)] = 1;
	map2[MAPPOS(0, 0)] = 1;

	for (i = 0; i < TOTALKNOTS; ++i) {
		knots[i].x = knots[i].y = 0;
	}

	total_moves = 0;
	while(fgets(buffer, BUFSIZE, fp)) {
		parse_move(buffer, moves + total_moves);
		++total_moves;
	}
	fclose(fp);

	for (i = 0; i < total_moves; ++i) {
		/* TODO this needs to happen step by step */
		apply_move(moves + i, knots);
		do {
			for (j = 1; j < TOTALKNOTS; ++j) {
				if (j == 1) {
					adjust_tail(map, knots + j - 1,
							knots + j);
				} else if (j == TOTALKNOTS - 1) {
					adjust_tail(map2, knots + j - 1,
							knots + j);
				} else {
					adjust_tail(NULL, knots + j - 1,
							knots + j);
				}
			}
		} while(apply_move(NULL, knots));
		/* print_map(moves + i, knots); */
	}

	/* print_visited(map2); */

	score = score2 = 0;
	for (i = 0; i < MAPSIZE; ++i) {
		score += map[i];
		score2 += map2[i];
	}

	free(map);
	free(map2);

	printf("%ld\n", score);
	printf("%ld\n", score2);

	return 0;
}
