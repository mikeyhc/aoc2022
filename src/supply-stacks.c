#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFSIZE 1024
#define MAX_STACKS 10
#define STACK_SIZE 128
#define MAX_INSTR 512

typedef struct {
	char stack[STACK_SIZE];
	int pos;
} Stack;

typedef struct {
	int num, source, dest;
} Instr;

void
init_stack(Stack *stack)
{
	stack->pos = 0;
}

void
init_stacks(Stack *stacks)
{
	int i;

	for (i = 0; i < MAX_STACKS; ++i) {
		init_stack(stacks + i);
	}
}

void
push_stack(Stack *stack, char c)
{
	stack->stack[stack->pos] = c;
	++(stack->pos);
	assert(stack->pos < STACK_SIZE);
}

char
pop_stack(Stack *stack)
{
	char c;

	c = stack->stack[stack->pos - 1];
	--(stack->pos);
	assert(stack->pos >= 0);

	return c;
}

char
peek_stack(Stack *stack)
{
	if (stack->pos == 0) {
		return 0;
	}

	return stack->stack[stack->pos - 1];
}

void
reverse_stack(Stack *stack)
{
	char temp;
	int i, j;

	for (i = 0, j = stack->pos - 1; i < j; ++i, --j) {
		temp = stack->stack[i];
		stack->stack[i] = stack->stack[j];
		stack->stack[j] = temp;
	}
}

int
parse_line(char *buffer, Stack *stacks)
{
	size_t pos;

	if (buffer[1] == '1') {
		return 0;
	}

	pos = 0;
	while (buffer[pos] != '\n' && buffer[pos] != '\0') {
		if (buffer[pos] == '[') {
			push_stack(stacks + (pos / 4), buffer[pos+1]);
		}

		pos += 4;
	}

	return 1;
}

void
read_stacks(FILE *fp, Stack *stacks)
{
	int i;
	char buffer[BUFSIZE];

	while (fgets(buffer, BUFSIZE, fp)) {
		if(!parse_line(buffer, stacks)) {
			/* eat the empty line */
			assert(fgets(buffer, BUFSIZE, fp) != 0);
			assert(buffer[0] == '\n');
			break;
		}
	}

	for (i = 0; i < MAX_STACKS; ++i) {
		reverse_stack(stacks + i);
	}
}

void
parse_instruction(char *buffer, Instr *instr)
{
	char *b;

	b = buffer + 5; /* skip "move " */
	instr->num = strtol(b, &b, 10);
	b = b + 6; /* skip " from " */
	instr->source = strtol(b, &b, 10);
	b = b + 4; /* skip " to " */
	instr->dest = strtol(b, &b, 10);
	assert(*b == '\0' || *b == '\n');
}

int
read_instructions(FILE *fp, Instr *instrs)
{
	int count;
	char buffer[BUFSIZE];

	count = 0;
	while (fgets(buffer, BUFSIZE, fp)) {
		parse_instruction(buffer, instrs + count);
		count++;
	}

	return count;
}

void
print_instr(Instr *instr)
{
	printf("{ num=%d; source=%d; dest=%d}\n", instr->num,
			instr->source, instr->dest);
}

void
print_stack(Stack *stack)
{
	int i;

	printf("[%d] ", stack->pos);
	for (i = 0; i < stack->pos; ++i) {
		printf("%c ", stack->stack[i]);
	}
	printf("\n");
}

void
print_stacks(Stack *stacks)
{
	int i;

	for (i = 0; i < MAX_STACKS; ++i) {
		print_stack(stacks + i);
	}
}

void
run_instruction(Instr *instr, Stack *stacks)
{
	int i;
	char c;

	for (i = 0; i < instr->num; ++i) {
		c = pop_stack(stacks + instr->source - 1);
		push_stack(stacks + instr->dest - 1, c);
	}
}

void
run_instruction2(Instr *instr, Stack *stacks)
{
	int i;
	Stack s;

	init_stack(&s);
	for (i = 0; i < instr->num; ++i) {
		push_stack(&s, pop_stack(stacks + instr->source - 1));
	}

	for (i = 0; i < instr->num; ++i) {
		push_stack(stacks + instr->dest - 1, pop_stack(&s));
	}
}

int
main(int argc, char **argv)
{
	FILE *fp;
	Stack stacks[MAX_STACKS], stacks2[MAX_STACKS];
	Instr instrs[MAX_INSTR];
	int instr_count;
	int i;
	char c;

	if (argc < 2) {
		fprintf(stderr, "usage: %s {FILE}\n", argv[0]);
		return 1;
	}

	fp = fopen(argv[1], "r");
	assert(fp);

	init_stacks(stacks);
	read_stacks(fp, stacks);
	instr_count = read_instructions(fp, instrs);
	fclose(fp);

	memcpy(stacks2, stacks, sizeof(stacks2));

	for (i = 0; i < instr_count; ++i) {
		run_instruction(instrs + i, stacks);
	}

	for (i = 0; i < instr_count; ++i) {
		run_instruction2(instrs + i, stacks2);
	}

	for (i = 0; i < MAX_STACKS; ++i) {
		c = peek_stack(stacks + i);
		if (c != 0) {
			printf("%c", c);
		}
	}
	printf("\n");

	for (i = 0; i < MAX_STACKS; ++i) {
		c = peek_stack(stacks2 + i);
		if (c != 0) {
			printf("%c", c);
		}
	}
	printf("\n");

	return 0;
}
