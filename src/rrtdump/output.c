/*
 * Copyright 2014-2017 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

/*
 * Output a plaintext dump of the railroad tnode tree
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../ast.h"

#include "../rrd/rrd.h"
#include "../rrd/pretty.h"
#include "../rrd/node.h"
#include "../rrd/list.h"
#include "../rrd/tnode.h"

#include "io.h"

static void
print_indent(FILE *f, int n)
{
	int i;

	assert(f != NULL);

	for (i = 0; i < n; i++) {
		fprintf(f, "    ");
	}
}

static void
print_coords(FILE *f, const struct tnode *n)
{
	fprintf(f, " %s", n->rtl ? "<-" : "->");
	fprintf(f, " w=%u a=%u d=%u", n->w, n->a, n->d);
}

static void
tnode_walk(FILE *f, const struct tnode *n, int depth)
{
	assert(f != NULL);
	assert(n != NULL);

	switch (n->type) {
		size_t i;

	case TNODE_SKIP:
		print_indent(f, depth);
		fprintf(f, "SKIP");
		print_coords(f, n);
		fprintf(f, "\n");

		break;

	case TNODE_ARROW:
		print_indent(f, depth);
		fprintf(f, "ARROW");
		print_coords(f, n);
		fprintf(f, "\n");

		break;

	case TNODE_ELLIPSIS:
		print_indent(f, depth);
		fprintf(f, "ELLIPSIS");
		print_coords(f, n);
		fprintf(f, "\n");

		break;

	case TNODE_CI_LITERAL:
		print_indent(f, depth);
		fprintf(f, "LITERAL");
		print_coords(f, n);
		fprintf(f, ": \"%s\"/i\n", n->u.literal);

		break;

	case TNODE_CS_LITERAL:
		print_indent(f, depth);
		fprintf(f, "LITERAL");
		print_coords(f, n);
		fprintf(f, ": \"%s\"\n", n->u.literal);

		break;

	case TNODE_LABEL:
		print_indent(f, depth);
		fprintf(f, "LABEL");
		print_coords(f, n);
		fprintf(f, ": %s\n", n->u.label);

		break;

	case TNODE_RULE:
		print_indent(f, depth);
		fprintf(f, "NAME");
		print_coords(f, n);
		fprintf(f, ": <%s>\n", n->u.name);

		break;

	case TNODE_VLIST:
		print_indent(f, depth);
		fprintf(f, "ALT");
		print_coords(f, n);
		fprintf(f, " o=%u", n->u.vlist.o); /* XXX: belongs in alt union */
		fprintf(f, ": [\n");
		for (i = 0; i < n->u.vlist.n; i++) {
			/* TODO: show tline for each alt */
			tnode_walk(f, n->u.vlist.a[i], depth + 1);
		}
		print_indent(f, depth);
		fprintf(f, "]\n");

		break;

	case TNODE_HLIST:
		print_indent(f, depth);
		fprintf(f, "SEQ");
		print_coords(f, n);
		fprintf(f, ": [\n");
		for (i = 0; i < n->u.hlist.n; i++) {
			tnode_walk(f, n->u.hlist.a[i], depth + 1);
		}
		print_indent(f, depth);
		fprintf(f, "]\n");

		break;
	}
}

static void
dim_string(const char *s, unsigned *w, unsigned *a, unsigned *d)
{
	assert(s != NULL);
	assert(w != NULL);
	assert(a != NULL);
	assert(d != NULL);

	*w = strlen(s); /* monospace */
	*a = 0;
	*d = 1;
}

void
rrtdump_output(const struct ast_rule *grammar)
{
	const struct ast_rule *p;

	for (p = grammar; p != NULL; p = p->next) {
		struct tnode *tnode;
		struct node *rrd;

		if (!ast_to_rrd(p, &rrd)) {
			perror("ast_to_rrd");
			return;
		}

		tnode = rrd_to_tnode(rrd, dim_string);

		if (!prettify) {
			printf("%s:\n", p->name);
			tnode_walk(stdout, tnode, 1);
			printf("\n");
		} else {
			printf("%s: (before prettify)\n", p->name);
			tnode_walk(stdout, tnode, 1);
			printf("\n");

			tnode_free(tnode);

			rrd_pretty(&rrd);

			tnode = rrd_to_tnode(rrd, dim_string);

			printf("%s: (after prettify)\n", p->name);
			tnode_walk(stdout, tnode, 1);
			printf("\n");
		}

		tnode_free(tnode);
		node_free(rrd);
	}
}

