#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define BUFSIZE 1024
#define MAXPACKETS 512

typedef enum {
	INT,
	LIST,
} Type;

typedef enum {
	LEFT,
	RIGHT,
	EQUAL,
} Order;

typedef struct list {
	Type type;
	union {
		struct list *list;
		int num;
	} data;
	struct list *next;
} List;


char*
parse_packet(char *buffer, List *packet)
{
	List *next;

	assert(*buffer == '['); /* ensure start of packet */
	++buffer;

	packet->next = NULL;
	while (*buffer != '\0' && *buffer != '\n') {
		switch (*buffer) {
		case '[':
			packet->type = LIST;
			packet->data.list = malloc(sizeof(List));
			assert(packet->data.list);
			buffer = parse_packet(buffer, packet->data.list);
			break;
		case ']': /* empty list */
			packet->type = LIST;
			packet->data.list = NULL;
			break;
		default:
			packet->type = INT;
			packet->data.num = strtol(buffer, &buffer, 10);
			assert(*buffer == ',' || *buffer == ']');
			break;
		}

		if (*buffer == ']') {
			++buffer;
			return buffer;
		} else {
			++buffer; /* eat , */
			next = malloc(sizeof(List));
			packet->next = next;
			packet = next;
			packet->next = NULL;
		}
	}

	return buffer;
}

void
print_packet(List *packet)
{
	putchar('[');
	while (packet != NULL) {
		switch (packet->type) {
		case INT:
			printf("%d", packet->data.num);
			break;
		case LIST:
			if (packet->data.list != NULL) {
				print_packet(packet->data.list);
			}
			break;
		default:
			assert(0);
		}
		if (packet->next != NULL) {
			putchar(',');
		}

		packet = packet->next;
	}
	putchar(']');
}

Order
list_cmp(List *a, List *b)
{
	Order order;
	List tmp;

	tmp.type = INT;
	tmp.next = NULL;
	while (a != NULL && b != NULL) {
		if (a->type == INT && b->type == INT) {
			if (a->data.num < b->data.num) {
				return LEFT;
			} else if (a->data.num > b->data.num) {
				return RIGHT;
			}
		} else if (a->type == LIST && b->type == LIST) {
			if (a->data.list == NULL && b->data.list == NULL) {
				return EQUAL;
			} else if (a->data.list == NULL &&
					b->data.list != NULL) {
				return LEFT;
			} else if (a->data.list != NULL &&
					b->data.list == NULL) {
				return RIGHT;
			}
			order = list_cmp(a->data.list, b->data.list);
			if (order == LEFT || order == RIGHT) {
				return order;
			}
		} else if (a->type == INT && b->type == LIST) {
			if (b->data.list == NULL) {
				return RIGHT;
			}
			tmp.data.num = a->data.num;
			order = list_cmp(&tmp, b->data.list);
			if (order == LEFT || order == RIGHT) {
				return order;
			}
		} else if (a->type == LIST && b->type == INT) {
			if (a->data.list == NULL) {
				return LEFT;
			}
			tmp.data.num = b->data.num;
			order = list_cmp(a->data.list, &tmp);
			if (order == LEFT || order == RIGHT) {
				return order;
			}
		} else {
			assert(0);
		}
		a = a->next;
		b = b->next;
	}

	if (a == NULL && b != NULL) {
		return LEFT;
	} else if (a == NULL && b == NULL) {
		return EQUAL;
	} else {
		return RIGHT;
	}
}

void
add_divider(List *packet, int val)
{
	List *child;

	packet->type = LIST;
	packet->data.list = malloc(sizeof(List));
	assert(packet->data.list);
	packet->next = NULL;

	child = packet->data.list;
	child->type = INT;
	packet->data.num = val;
	child->next = NULL;
}

void
merge(List *packets[], int start, int mid, int end)
{
	List *val;
	int start2, idx;
	Order order;

	start2 = mid + 1;
	order = list_cmp(packets[mid], packets[start2]);
	if (order == LEFT || order == EQUAL) {
		/* list is already sorted */
		return;
	}

	while (start <= mid && start2 <= end) {
		order = list_cmp(packets[start], packets[start2]);
		if (order == LEFT || order == EQUAL) {
			++start;
		} else {
			val = packets[start2];
			idx = start2;

			while (idx != start) {
				packets[idx] = packets[idx - 1];
				--idx;
			}
			packets[start] = val;

			++start;
			++mid;
			++start2;
		}
	}
}

void
merge_sort(List *packets[], int left, int right)
{
	int mid;

	if (right <= left) {
		return;
	}

	mid = left + (right - left) / 2;
	merge_sort(packets, left, mid);
	merge_sort(packets, mid + 1, right);
	merge(packets, left, mid, right);
}

int
is_divider(List *packet)
{
	List *child;

	if (packet->type != LIST || packet->next != NULL) {
		return 0;
	}

	if (packet->data.list ==  NULL) {
		return 0;
	}

	child = packet->data.list;
	if (child->type != INT || child->next != NULL) {
		return 0;
	}

	return child->data.num == 2 || child->data.num == 6;
}

int
dividers_product(List *packets[], int max_packets)
{
	int score = -1;
	int i = 0;

	for (i = 0; i < max_packets; ++i) {
		if (is_divider(packets[i])) {
			if (score < 0) {
				score = i + 1;
			} else {
				return score * (i + 1);
			}
		}
	}

	assert(0);
}

void
free_list(List *list)
{
	List *next;

	while (list != NULL) {
		if (list->type == LIST) {
			free_list(list->data.list);
		}
		next = list->next;
		free(list);
		list = next;
	}
}

int
main(int argc, char **argv)
{
	char buffer[BUFSIZE];
	FILE *fp;
	List *packets[MAXPACKETS];
	int packet_count;
	int i, pair_idx, sum;
	Order order;

	if (argc < 2) {
		fprintf(stderr, "usage: %s {FILE}\n", argv[0]);
		return 1;
	}

	fp = fopen(argv[1], "r");
	assert(fp);

	packet_count = 0;
	while (fgets(buffer, BUFSIZE, fp)) {
		if (buffer[0] == '\n') {
			continue;
		}
		packets[packet_count] = malloc(sizeof(List));
		assert(packets[packet_count]);
		parse_packet(buffer, packets[packet_count]);
		++packet_count;
	}
	fclose(fp);

	assert(packet_count % 2 == 0);

	sum = 0;
	pair_idx = 1;
	for (i = 0; i < packet_count; i += 2) {
		order = list_cmp(packets[i], packets[i + 1]);
		assert(order != EQUAL);
		if (order == LEFT) {
			sum += pair_idx;
		}
		++pair_idx;
	}

	printf("%d\n", sum);

	packets[packet_count] = malloc(sizeof(List));
	assert(packets[packet_count]);
	parse_packet("[[2]]", packets[packet_count]);
	++packet_count;

	packets[packet_count] = malloc(sizeof(List));
	assert(packets[packet_count]);
	parse_packet("[[6]]", packets[packet_count]);
	++packet_count;

	merge_sort(packets, 0, packet_count - 1);

	printf("%d\n", dividers_product(packets, packet_count));

	for (i = 0; i < packet_count; ++i) {
		free_list(packets[i]);
	}

	return 0;
}
