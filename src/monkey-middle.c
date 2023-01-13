#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFSIZE 1024
#define MAXQUEUE 64
#define MAXMONKEY 8
#define MAXROUNDS 20
#define MAXERROUNDS 10000

typedef enum {
	OLD,
	FIXED,
} TermType;

typedef struct {
	TermType t;
	long v;
} Term;

typedef enum {
	ADD,
	MULT,
} Operator;

typedef struct {
	Term left, right;
	Operator op;
} Operation;

typedef struct {
	int id;
	long items[MAXQUEUE];
	int item_count;
	Operation op;
	int test;
	int true_monkey, false_monkey;
	int inspections;
} Monkey;

void
parse_term(char *buffer, Term *term)
{
	if (strncmp("old", buffer, 3) == 0) {
		term->t = OLD;
	} else {
		term->t = FIXED;
		term->v = strtol(buffer, NULL, 10);
		assert(errno != ERANGE);
	}
}

Operator
parse_op(char *buffer)
{
	switch (*buffer) {
	case '+':
		return ADD;
	case '*':
		return MULT;
	default:
		assert(0);
	}
}

void
parse_monkey(Monkey *monkey, FILE *fp, char *buffer)
{
	char *token;

	monkey->id = strtol(buffer + 7, NULL, 10);

	/* read items */
	assert(fgets(buffer, BUFSIZE, fp));
	strtok(buffer, " "); /* eat "Starting" */
	strtok(NULL, " "); /* eat "items:" */
	monkey->item_count = 0;
	while ((token = strtok(NULL, " ")) != NULL) {
		monkey->items[monkey->item_count] = strtol(token, NULL, 10);
		assert(errno != ERANGE);
		++monkey->item_count;
	}

	/* read operation */
	assert(fgets(buffer, BUFSIZE, fp));
	strtok(buffer, " "); /* eat "Operation:" */
	strtok(NULL, " "); /* eat "new" */
	strtok(NULL, " "); /* eat "=" */
	parse_term(strtok(NULL, " "), &monkey->op.left);
	monkey->op.op = parse_op(strtok(NULL, " "));
	parse_term(strtok(NULL, " "), &monkey->op.right);

	/* test */
	assert(fgets(buffer, BUFSIZE, fp));
	strtok(buffer, " "); /* eat "Test:" */
	strtok(NULL, " "); /* eat "divisible" */
	strtok(NULL, " "); /* eat "by" */
	monkey->test = strtol(strtok(NULL, " "), NULL, 0);

	/* true */
	assert(fgets(buffer, BUFSIZE, fp));
	strtok(buffer, " "); /* eat "if" */
	strtok(NULL, " "); /* eat "true:" */
	strtok(NULL, " "); /* eat "throw" */
	strtok(NULL, " "); /* eat "to" */
	strtok(NULL, " "); /* eat "monkey" */
	monkey->true_monkey = strtol(strtok(NULL, " "), NULL, 0);

	/* false */
	assert(fgets(buffer, BUFSIZE, fp));
	strtok(buffer, " "); /* eat "if" */
	strtok(NULL, " "); /* eat "false:" */
	strtok(NULL, " "); /* eat "throw" */
	strtok(NULL, " "); /* eat "to" */
	strtok(NULL, " "); /* eat "monkey" */
	monkey->false_monkey = strtol(strtok(NULL, " "), NULL, 0);

	monkey->inspections = 0;
	fgets(buffer, BUFSIZE, fp); /* eat empty line */
}

void
print_monkey(Monkey *monkey)
{
	int i;

	printf("Monkey %d:\n", monkey->id);
	printf("  Starting items: ");
	for (i = 0; i < monkey->item_count; ++i) {
		if (i > 0) {
			printf(", ");
		}
		printf("%ld", monkey->items[i]);
	}
	printf("\n");
	printf("  Operation: new = ");
	if (monkey->op.left.t == OLD) {
		printf("old ");
	} else {
		printf("%ld ", monkey->op.left.v);
	}
	printf("%c", monkey->op.op == ADD ? '+' : '*');
	if (monkey->op.right.t == OLD) {
		printf(" old\n");
	} else {
		printf(" %ld\n", monkey->op.right.v);
	}
	printf("  Test: divisible by %d\n", monkey->test);
	printf("    If true: throw to monkey %d\n",
			monkey->true_monkey);
	printf("    If false: throw to monkey %d\n",
			monkey->false_monkey);
	printf("  Inspections: %d\n\n", monkey->inspections);
}

long
run_op(Operation *op, long old, long stress_loss)
{
	long left, right;

	left = op->left.t == OLD ? old : op->left.v;
	right = op->right.t == OLD ? old : op->right.v;

	if (op->op == ADD) {
		return (left + right) / stress_loss;
	} else if (op->op == MULT) {
		return (left * right) / stress_loss;
	} else {
		assert(0);
	}
}

void
push_item(Monkey *monkey, long item)
{
	monkey->items[monkey->item_count] = item;
	++monkey->item_count;
}

void
run_monkey(Monkey *monkey, Monkey *others, long stress_loss, long divisor)
{
	int i;
	long new_item;

	for (i = 0; i < monkey->item_count; ++i) {
		new_item = run_op(&monkey->op, monkey->items[i], stress_loss);
		new_item = new_item % divisor;
		++monkey->inspections;
		if (new_item % monkey->test == 0) {
			push_item(others + monkey->true_monkey, new_item);
		} else {
			push_item(others + monkey->false_monkey, new_item);
		}
	}
	monkey->item_count = 0;
}

void
run_monkies(Monkey *monkies, int monkey_count, int max_rounds, long stress_loss)
{
	int i, rounds;
	long max, second, divisor;

	divisor = 1;
	for (i = 0; i < monkey_count; ++i) {
		divisor *= monkies[i].test;
	}

	for (rounds = 0; rounds < max_rounds; ++rounds) {
		for (i = 0; i < monkey_count; ++i) {
			run_monkey(monkies + i, monkies, stress_loss, divisor);
		}
	}

	max = second = 0;
	for (i = 0; i < monkey_count; ++i) {
		if (monkies[i].inspections > max) {
			second = max;
			max = monkies[i].inspections;
		} else if (monkies[i].inspections > second) {
			second = monkies[i].inspections;
		}
	}

	printf("%ld\n", max * second);
}

int
main(int argc, char **argv)
{
	FILE *fp;
	char buffer[BUFSIZE];
	Monkey monkies[MAXMONKEY], more_monkies[MAXMONKEY];
	int monkey_count;

	if (argc < 2) {
		fprintf(stderr, "usage: %s {FILE}\n", argv[0]);
		return 1;
	}

	fp = fopen(argv[1], "r");
	assert(fp);

	monkey_count = 0;
	while(fgets(buffer, BUFSIZE, fp)) {
		parse_monkey(monkies + monkey_count, fp, buffer);
		++monkey_count;
	}
	fclose(fp);
	memcpy(more_monkies, monkies, sizeof(Monkey) * MAXMONKEY);

	run_monkies(monkies, monkey_count, MAXROUNDS, 3);
	run_monkies(more_monkies, monkey_count, MAXERROUNDS, 1);

	return 0;
}
