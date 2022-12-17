#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFSIZE 1024
#define NAMESIZE 16
#define SIZELIMIT 100000
#define TOTALSPACE 70000000
#define REQSPACE 30000000

typedef enum {
	T_FILE,
	T_DIR
} Type;

typedef struct {
	struct file_ent *children;
	struct file_ent *parent;
} DirData;

typedef struct file_ent {
	char name[NAMESIZE];
	Type type;
	union { long size; DirData dir; } data;
	struct file_ent *next;
} FileEnt;

void
init_dir(FileEnt *dir, char *name, FileEnt *parent)
{
	strncpy(dir->name, name, NAMESIZE);
	dir->type = T_DIR;
	dir->data.dir.children = NULL;
	dir->data.dir.parent = parent;
	dir->next = NULL;
}

void
init_file(FileEnt *file, char *name, long size)
{
	strncpy(file->name, name, NAMESIZE);
	file->type = T_FILE;
	file->data.size = size;
	file->next = NULL;
}


FileEnt*
find_dir(FileEnt *cwd, char *name)
{
	FileEnt *r;

	r = cwd->data.dir.children;
	while (r != NULL) {
		if (strncmp(r->name, name, NAMESIZE) == 0) {
			return r;
		}

		r = r->next;

	}

	assert(0);
}

void
append_file(FileEnt *dir, FileEnt *file)
{
	FileEnt *cur;

	cur = dir->data.dir.children;

	if (cur == NULL) {
		dir->data.dir.children = file;
		return;
	}

	while (cur->next != NULL) {
		cur = cur->next;
	}

	cur->next = file;
}

void
print_dir(FileEnt *dir, long space)
{
	int i;
	FileEnt *child;

	for (i = 0; i < space; ++i) putchar(' ');
	space += 2;
	printf("- %s (dir)\n", dir->name);

	for (child = dir->data.dir.children; child != NULL;
			child = child->next) {
		if (child->type == T_DIR) {
			print_dir(child, space);
		} else {
			for (i = 0; i < space; ++i) putchar(' ');
			printf("- %s (file, size=%ld)\n",
					child->name,
					child->data.size);
		}
	}
}

void
rtrim(char *s) {
	while (*s != '\0') ++s;
	--s;
	while (isspace(*s)) --s;
	*(s + 1) = '\0';
}

void
sum_sizes(FileEnt *tree, long *total, long *valid)
{
	long child_total, child_valid;
	FileEnt *child;

	*total = *valid = 0;
	for (child = tree->data.dir.children; child; child = child->next) {
		if (child->type == T_DIR) {
			child_total = child_valid = 0;
			sum_sizes(child, &child_total, &child_valid);
			*total += child_total;
			*valid += child_valid;
			if (child_total < SIZELIMIT) {
				*valid += child_total;
			}
		} else {
			*total += child->data.size;
		}
	}
}

long
smallest_delete(FileEnt *tree, long threshold, long *total)
{
	long smallest = INT_MAX, ctotal, tmp;
	FileEnt *e;

	*total = 0;
	for (e = tree->data.dir.children; e; e = e->next) {
		if (e->type == T_FILE) {
			*total += e->data.size;
		} else {
			tmp = smallest_delete(e, threshold, &ctotal);
			if (tmp < smallest && tmp >= threshold) {
				smallest = tmp;
			} else if (ctotal < smallest && ctotal >= threshold) {
				smallest = ctotal;
			}
			*total += ctotal;
		}
	}

	return smallest;
}

void
free_dir(FileEnt *dir)
{
	FileEnt *e, *n;

	for (e = dir->data.dir.children; e; e = n) {
		n = e->next;
		if (e->type == T_DIR) {
			free_dir(e);
		}
		free(e);
	}
}

int
main(int argc, char **argv)
{
	FILE *fp;
	char buffer[BUFSIZE], *rem;
	FileEnt root, *cwd, *tmp;
	long size, total, valid, reqsize, smallest, stotal;

	if (argc < 2) {
		fprintf(stderr, "usage: %s {FILE}\n", argv[0]);
		return 1;
	}

	fp = fopen(argv[1], "r");
	assert(fp);

	init_dir(&root, "/", NULL);
	cwd = &root;
	while(fgets(buffer, BUFSIZE, fp)) {
		rtrim(buffer);
		if (buffer[0] == '$') {
			/* skip ls */
			if (buffer[2] == 'l') {
				continue;
			}

			if (buffer[5] == '/') {
				cwd = &root;
			} else if (buffer[5] == '.' && buffer[6] == '.') {
				cwd = cwd->data.dir.parent;
			} else {
				cwd = find_dir(cwd, buffer + 5);
			}
		} else {
			tmp = malloc(sizeof(FileEnt));
			assert(tmp);
			if (buffer[0] == 'd') {
				init_dir(tmp, buffer + 4, cwd);
			} else {
				size = strtol(buffer, &rem, 10);
				init_file(tmp, rem + 1, size);
			}
			append_file(cwd, tmp);
		}
	}
	fclose(fp);

	sum_sizes(&root, &total, &valid);
	reqsize = REQSPACE - (TOTALSPACE - total);
	smallest = smallest_delete(&root, reqsize, &stotal);

	printf("%ld\n", valid);
	printf("%ld\n", smallest);

	free_dir(&root);

	return 0;
}
