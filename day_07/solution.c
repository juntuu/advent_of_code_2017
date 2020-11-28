#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void *alloc(size_t size)
{
	void *p = calloc(1, size);
	if (!p)
		exit(1);
	return p;
}

struct List {
	struct Node *value;
	struct List *next;
};

struct Node {
	const char *name;
	int weight;
	int _weight;
	struct Node *in;
	struct List *out;
};

struct List *list_add(struct List *head, struct Node *value)
{
	struct List *p = alloc(sizeof(*p));
	p->value = value;
	p->next = head;
	return p;
}

int node_weight(struct Node *n)
{
	if (!n)
		return 0;
	if (n->_weight)
		return n->_weight;
	int w = n->weight;
	for (struct List *p = n->out; p; p = p->next)
		w += node_weight(p->value);
	n->_weight = w;
	return w;
}

struct Node *node_entry(struct List **nodes, const char *name)
{
	for (struct List *p = *nodes; p; p = p->next)
		if (strcmp(p->value->name, name) == 0)
			return p->value;

	struct Node *node = alloc(sizeof(*node));
	node->name = strdup(name);
	*nodes = list_add(*nodes, node);
	return node;
}

struct Node *node_imbalance(struct Node *n)
{
	if (!n || !n->out)
		return NULL;
	int expect = node_weight(n->out->value);
	for (struct List *p = n->out; p; p = p->next) {
		struct Node *x = node_imbalance(p->value);
		if (x)
			return x;
		if (expect != node_weight(p->value))
			return n;
	}
	return NULL;
}

struct Node *node_balanced(struct Node *n)
{
	if (!n)
		return NULL;
	struct Node *out[3];
	int i = 0;
	struct List *p = n->out;
	int diff = 0;
	for (; i < 3; p = p->next, i++) {
		if (!p) {
			assert(i > 0);
			diff = abs(out[1]->_weight - out[0]->_weight);
			if (out[1]->_weight < out[0]->_weight) {
				out[0] = out[1];
			}
			goto success;
		}
		out[i] = p->value;
	}
	int expect;
	if (out[0]->_weight == out[1]->_weight && out[1]->_weight == out[2]->_weight) {
		expect = out[0]->_weight;
		for (; p; p = p->next) {
			out[0] = p->value;
			if (out[0]->_weight != expect)
				break;
		}
	} else if (out[0]->_weight == out[1]->_weight) {
		expect = out[0]->_weight;
		out[0] = out[2];
	} else if (out[0]->_weight == out[2]->_weight) {
		expect = out[0]->_weight;
		out[0] = out[1];
	} else if (out[1]->_weight == out[2]->_weight) {
		expect = out[1]->_weight;
	} else {
		exit(1);
	}
	diff = abs(expect - out[0]->_weight);
	if (expect < out[0]->_weight)
		diff *= -1;
success:
	assert(diff != 0);
	n->_weight += diff;
	out[0]->weight += diff;
	out[0]->_weight += diff;
	assert(out[0]->weight >= 0);
	return out[0];
}

struct List *parse_line(struct List *nodes, char *line, ssize_t n)
{
	if (line[n-1] == '\n')
		line[n-1] = '\0';
	char *name = strsep(&line, " ");

	struct Node *p = node_entry(&nodes, name);
	sscanf(line, " (%d)", &p->weight);

	strsep(&line, ">");

	while (line) {
		while (*line == ' ')
			line++;
		char *edge = strsep(&line, ",");
		struct Node *q = node_entry(&nodes, edge);
		q->in = p;
		p->out = list_add(p->out, q);
	}
	return nodes;
}

int main(void)
{
	struct List *nodes = NULL;
	char *buf = NULL;
	size_t cap = 0;
	for (ssize_t n; (n = getline(&buf, &cap, stdin)) > 0;)
		nodes = parse_line(nodes, buf, n);
	if (!nodes)
		return 1;

	struct Node *root = NULL;
	for (struct List *it = nodes; it; it = it->next) {
		root = it->value;
		if (!root->in)
			break;
	}
	if (!root || root->in)
		return 1;
	puts(root->name);
	struct Node *p = node_balanced(node_imbalance(root));
	if (!p)
		return 1;
	printf("%d\n", p->weight);
	assert(node_imbalance(root) == NULL);
}
