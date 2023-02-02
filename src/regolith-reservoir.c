#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFSIZE 1024
#define MAPHEIGHT 1024
#define MAPWIDTH MAPHEIGHT
#define MAPCELLS (MAPHEIGHT * MAPWIDTH)
#define ROCK '#'
#define SAND 'o'
#define AIR '.'

typedef struct {
	int minx, miny;
	int maxx, maxy;
	char map[MAPCELLS];
} Map;

/* start: map functions */

void
init_map(Map *map)
{
	memset(map->map, AIR, MAPCELLS);
	map->minx = MAPWIDTH;
	map->miny = MAPHEIGHT;
	map->maxx = 0;
	map->miny = 0;
}

char
map_get(Map *map, int x, int y)
{
	return map->map[y * MAPHEIGHT + x];
}

void
map_set_x_limit(Map *map, int x)
{
	if (x < map->minx) {
		map->minx = x;
	}
	if (x > map->maxx) {
		map->maxx = x;
	}
}

void
map_set_y_limit(Map *map, int y)
{
	if (y < map->miny) {
		map->miny = y;
	}
	if (y > map->maxy) {
		map->maxy = y;
	}
}

void
map_set_limits(Map *map, int x, int y)
{
	map_set_x_limit(map, x);
	map_set_y_limit(map, y);
}

void
map_put_rock(Map *map, int x, int y)
{
	map_set_limits(map, x, y);
	map->map[y * MAPHEIGHT + x] = ROCK;
}

void
map_put_sand(Map *map, int x, int y)
{
	map_set_x_limit(map, x);
	map->map[y * MAPHEIGHT + x] = SAND;
}

int
map_has_blocker(Map *map, int x, int y)
{
	return map_get(map, x, y) != AIR;
}

void
print_map(Map *map)
{
	int col, row;
	int startx, starty, endx, endy;

	startx = map->minx > 0 ? map->minx - 1 : 0;
	endx = map->maxx + 2 < MAPWIDTH ? map->maxx + 2 : MAPWIDTH;
	starty = map->miny > 0 ? map->miny - 1 : 0;
	endy = map->maxy + 2 < MAPHEIGHT ? map->maxy + 2 : MAPHEIGHT;
	printf("(%d,%d)/(%d,%d)\n", startx, starty, endx, endy);
	for (row = starty; row < endy; ++row) {
		for (col = startx; col < endx; ++col) {
			putchar(map_get(map, col, row));
		}
		putchar('\n');
	}
}

void
print_blocked_map(Map *map)
{
	int i;

	print_map(map);
	for (i = map->minx; i < map->maxx + 3; ++i) {
		putchar('#');
	}
	putchar('\n');
}

/* end: map functions */

void
parse_line(char *input, Map *map)
{
	char *tok, *rest;
	int x, y, oldx, oldy, row, col;
	int startx, endx, starty, endy;

	tok = strtok(input, " -> ");
	oldx = strtol(tok, &rest, 10);
	oldy = strtol(rest + 1, NULL, 10);

	tok = strtok(NULL, " -> ");
	while (tok != NULL && *tok != '\n') {
		x = strtol(tok, &rest, 10);
		y = strtol(rest + 1, NULL, 10);


		if (oldx < x) {
			startx = oldx;
			endx = x;
		} else {
			startx = x;
			endx = oldx;
		}
		if (oldy < y) {
			starty = oldy;
			endy = y;
		} else {
			starty = y;
			endy = oldy;
		}

		for (row = starty; row <= endy; ++row) {
			for (col = startx; col <= endx; ++col) {
				map_put_rock(map, col, row);
			}
		}

		oldx = x;
		oldy = y;
		tok = strtok(NULL, " -> ");
	}
}

int
drop_sand(Map *map)
{
	int sandx, sandy;

	sandx = 500;
	sandy = 0;

	while (sandy < map->maxy + 1)  {
		if (!map_has_blocker(map, sandx, sandy+1)) {
			++sandy;
		} else if (!map_has_blocker(map, sandx-1, sandy+1)) {
			--sandx;
			++sandy;
		} else if (!map_has_blocker(map, sandx+1, sandy+1)) {
			++sandx;
			++sandy;
		} else {
			map_put_sand(map, sandx, sandy);
			return 0;
		}
	}

	return 1;
}

int
drop_blocked_sand(Map *map)
{
	int sandx, sandy;

	sandx = 500;
	sandy = 0;

	do {
		if (sandy == map->maxy + 1) {
			map_put_sand(map, sandx, sandy);
			return 0;
		} else if (!map_has_blocker(map, sandx, sandy+1)) {
			++sandy;
		} else if (!map_has_blocker(map, sandx-1, sandy+1)) {
			--sandx;
			++sandy;
		} else if (!map_has_blocker(map, sandx+1, sandy+1)) {
			++sandx;
			++sandy;
		} else {
			map_put_sand(map, sandx, sandy);
			return sandx == 500 && sandy == 0;
		}
	} while (sandx != 500 || sandy != 0);

	return 1;
}

int
run_sim(Map *map)
{
	int steps = 0;

	/* add a limit */
	while (steps < INT_MAX) {
		if (drop_sand(map)) {
			break;
		}
		++steps;
	}

	assert(steps != INT_MAX);

	return steps;
}

int
run_blocked_sim(Map *map)
{
	int steps = 0;

	/* add a limit */
	while (steps < INT_MAX) {
		++steps;
		if (drop_blocked_sand(map)) {
			break;
		}
	}

	assert(steps != INT_MAX);

	return steps;
}

int
main(int argc, char **argv)
{
	char buffer[BUFSIZE];
	FILE *fp;
	Map map;
	int steps;

	if (argc < 2) {
		fprintf(stderr, "usage: %s {FILE}\n", argv[0]);
		return 1;
	}

	fp = fopen(argv[1], "r");
	assert(fp);

	init_map(&map);

	while (fgets(buffer, BUFSIZE, fp)) {
		parse_line(buffer, &map);
	}
	fclose(fp);

	steps = run_sim(&map);
	printf("%d\n", steps);
	printf("%d\n", steps + run_blocked_sim(&map));

	return 0;
}
