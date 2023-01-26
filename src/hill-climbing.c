#include <assert.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFSIZE 1024
#define MAPWIDTH 70
#define MAPHEIGHT 42
#define MAPCELLS (MAPWIDTH * MAPHEIGHT)
#define MAXNEIGHBORS 4
#define MAXPRIORITY (MAPCELLS + 'z' - 'a')
#define START 'S'
#define END 'E'

typedef struct {
	unsigned x, y;
} Point;

typedef struct {
	char map[MAPCELLS];
	Point start, end;
	size_t height, width;
} Map;


typedef struct {
	Point p;
	unsigned s;
} ScoredPoint;

typedef struct ll_node {
	ScoredPoint *data;
	struct ll_node *next;
} LLNode;

typedef struct {
	LLNode *buckets[MAXPRIORITY];
} PQueue;

typedef struct {
	unsigned cells[MAPCELLS];
} PointMap;

/* start: map functions */

unsigned pm_get(PointMap*, Point*);

void
print_map(Map *map)
{
	unsigned col, row;

	for (row = 0; row < map->height; ++row) {
		for (col = 0; col < map->width; ++col) {
			if (col == map->start.x && row == map->start.y) {
				putchar('S');
			} else if (col == map->end.x && row == map->end.y) {
				putchar('E');
			} else {
				putchar(map->map[(row * map->width + col)]);
			}
		}
		putchar('\n');
	}
	putchar('\n');
	printf("START: (%d,%d)\n", map->start.x, map->start.y);
	printf("END:   (%d,%d)\n", map->end.x, map->end.y);
}

void
print_visited(Map *map, PointMap *visited)
{
	unsigned col, row;
	Point p;

	for (row = 0; row < map->height; ++row) {
		for (col = 0; col < map->width; ++col) {
			p.x = col;
			p.y = row;
			if (col == map->start.x && row == map->start.y) {
				putchar('@');
			} else if (col == map->end.x && row == map->end.y) {
				putchar('@');
			} else if (pm_get(visited, &p) != UINT_MAX) {
				putchar(map->map[(row * map->width + col)]
						- 'a' + 'A');
			} else {
				putchar(map->map[(row * map->width + col)]);
			}
		}
		putchar('\n');
	}
	putchar('\n');
	printf("START: (%d,%d)\n", map->start.x, map->start.y);
	printf("END:   (%d,%d)\n", map->end.x, map->end.y);
}

char
map_get(Map *map, Point *p)
{
	return map->map[(p->y * map->width) + p->x];
}

void
read_map(Map *map, FILE *fp)
{
	size_t offset;
	unsigned i;

	map->height = map->width = 0;
	while (fgets(map->map + (map->height * map->width), BUFSIZE, fp)) {
		offset = map->height * map->width;
		if (map->width == 0) {
			map->width = strlen(map->map) - 1;
		}
		for (i = 0; i < map->width; ++i) {
			if (map->map[offset + i] == START) {
				map->start.x = i;
				map->start.y = map->height;
				map->map[offset + i] = 'a';
			} else if (map->map[offset + i] == END) {
				map->end.x = i;
				map->end.y = map->height;
				map->map[offset + i] = 'z';
			}
		}
		++map->height;
	}
}

/* end: map functions */

/* start: pqueue functions */

void
init_pqueue(PQueue *queue)
{
	int i;

	for (i = 0; i < MAXPRIORITY; ++i) {
		queue->buckets[i] = NULL;
	}
}

void
pqueue_push(PQueue *queue, unsigned prio, ScoredPoint *data)
{
	LLNode *new, *c;

	new = malloc(sizeof(LLNode));
	assert(new);
	new->data = data;
	new->next = NULL;

	if (queue->buckets[prio] == NULL) {
		queue->buckets[prio] = new;
		return;
	}

	/* find end of list */
	assert(prio < MAXPRIORITY);
	for (c = queue->buckets[prio]; c->next != NULL; c = c->next) ;
	c->next = new;
}

ScoredPoint*
pqueue_pop(PQueue *queue)
{
	int i;
	LLNode *n;
	ScoredPoint *p;

	for (i = 0; i < MAXPRIORITY; ++i) {
		if (queue->buckets[i] != NULL) {
			n = queue->buckets[i];
			queue->buckets[i] = n->next;
			p = n->data;
			free(n);
			return p;
		}
	}

	return NULL;
}

void
free_pqueue(PQueue *queue)
{
	ScoredPoint *p;

	while ((p = pqueue_pop(queue)) != NULL) {
		free(p);
	}
}

/* end: pqueue functions */

/* start: pointmap functions */

void
init_pointmap(PointMap *map)
{
	int i;

	for (i = 0; i < MAPCELLS; ++i) {
		map->cells[i] = UINT_MAX;
	}
}

unsigned
pm_get(PointMap *map, Point *p)
{
	return map->cells[(p->y * MAPWIDTH) + p->x];
}

void
pm_insert(PointMap *map, Point *p, unsigned score)
{
	map->cells[(p->y * MAPWIDTH) + p->x] = score;
}

/* end: pointmap functions */

int
is_end(Map *map, Point *p)
{
	return p->x == map->end.x && p->y == map->end.y;
}

unsigned
score_point(Map *map, Point *dest)
{
	unsigned score;

	score = dest->x < map->end.x ? map->end.x - dest->x
		: dest->x - map->end.x;
	score += dest->y < map->end.y ? map->end.y - dest->y
		: dest->y - map->end.y;
	score += 'z'  - map_get(map, dest);

	return score;
}

int
valid_move(Map *map, Point *a, Point *b)
{
	return b->x < map->width &&
		b->y < map->height &&
		map_get(map, b) - map_get(map, a) < 2;
}

ScoredPoint*
get_neighbor(ScoredPoint *source, Map *map)
{
	static ScoredPoint *stored, *neighbors[4], *r;
	static int i;

	if (stored != source) {
		stored = source;

		for (i = 0; i < MAXNEIGHBORS; ++i) {
			neighbors[i] = malloc(sizeof(ScoredPoint));
			assert(neighbors[i]);
		}

		neighbors[0]->p.x = stored->p.x;
		neighbors[0]->p.y = stored->p.y - 1;
		neighbors[0]->s = stored->s + 1;

		neighbors[1]->p.x = stored->p.x - 1;
		neighbors[1]->p.y = stored->p.y;
		neighbors[1]->s = stored->s + 1;

		neighbors[2]->p.x = stored->p.x + 1;
		neighbors[2]->p.y = stored->p.y;
		neighbors[2]->s = stored->s + 1;

		neighbors[3]->p.x = stored->p.x;
		neighbors[3]->p.y = stored->p.y + 1;
		neighbors[3]->s = stored->s + 1;

		i = 0;
	}

	while (i < MAXNEIGHBORS &&
			!valid_move(map, &stored->p, &neighbors[i]->p)) {
		free(neighbors[i]);
		++i;
	}

	if (i >= MAXNEIGHBORS) {
		return NULL;
	}

	r = neighbors[i];
	++i;
	return r;
}

unsigned
climb_hill(Map *map)
{
	PQueue to_visit;
	PointMap visited;
	ScoredPoint *current, *neighbor;
	unsigned score, steps;

	init_pqueue(&to_visit);
	init_pointmap(&visited);

	current = malloc(sizeof(ScoredPoint));
	assert(current);
	current->p.x = map->start.x;
	current->p.y = map->start.y;
	current->s = 0;

	while (current != NULL) {
		if (is_end(map, &current->p)) {
			break;
		}

		/* skip if visited at a lower step count */
		if (pm_get(&visited, &current->p) <= current->s) {
			goto ch_cleanup;
		}

		pm_insert(&visited, &current->p, current->s);

		while ((neighbor = get_neighbor(current, map)) != NULL) {
			if (pm_get(&visited, &neighbor->p) <= neighbor->s) {
				free(neighbor);
				continue;
			}
			score = score_point(map, &neighbor->p);
			pqueue_push(&to_visit, neighbor->s + score, neighbor);
		}

ch_cleanup:
		free(current);
		current = pqueue_pop(&to_visit);
		if (current == NULL) {
			return UINT_MAX;
		}
	}

	free_pqueue(&to_visit);

	steps = current->s;
	free(current);

	return steps;
}

int
main(int argc, char **argv)
{
	FILE *fp;
	Map map;
	Point start;
	unsigned steps, min_steps;

	if (argc < 2) {
		fprintf(stderr, "usage: %s {FILE}\n", argv[0]);
		return 1;
	}

	fp = fopen(argv[1], "r");
	assert(fp);

	read_map(&map, fp);
	fclose(fp);

	printf("%u\n", climb_hill(&map));

	min_steps = UINT_MAX;
	for (start.y = 0; start.y < map.height; ++start.y) {
		for (start.x = 0; start.x < map.width; ++start.x) {
			if (map_get(&map, &start) == 'a') {
				map.start.x = start.x;
				map.start.y = start.y;
				steps = climb_hill(&map);
				min_steps = steps < min_steps ? steps
					: min_steps;
			}
		}
	}

	printf("%u\n", min_steps);

	return 0;
}
