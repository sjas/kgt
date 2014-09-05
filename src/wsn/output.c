/* $Id$ */

/*
 * Wirth Syntax Notation Output.
 *
 * TODO: fprintf(fout), instead of stdout
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "../ast.h"

#include "io.h"

static void output_alt(struct ast_alt *alt);

static void
output_term(struct ast_term *term)
{
	const char *s, *e;
	size_t i;

	struct {
		unsigned int min;
		unsigned int max;
		const char *s;
		const char *e;
	} a[] = {
		{ 1, 1, " ( ", " )" },
		{ 1, 1, "",    ""   },
		{ 0, 1, " [ ", " ]" },
		{ 0, 0, " { ", " }" }
	};

	s = NULL;
	e = NULL;

	for (i = 0; i < sizeof a / sizeof *a; i++) {
		if (i == 0 && term->type != TYPE_GROUP) {
			continue;
		}

		if (term->min == a[i].min && term->min == a[i].min) {
			s = a[i].s;
			e = a[i].e;
			break;
		}
	}

	/* TODO: for {1,0} output first term inline */

	assert(s != NULL && e != NULL);

	/* EBNF cannot express minimum term repetition; TODO: semantic checks for this */
	assert(term->min <= 1);
	assert(term->max <= 1);

	printf("%s", s);

	switch (term->type) {
	case TYPE_EMPTY:
		fputs(" \"\"", stdout);
		break;

	case TYPE_PRODUCTION:
		printf(" %s", term->u.name);
		break;

	case TYPE_TERMINAL: {
			const char *p;

			fputs(" \"", stdout);
			for (p = term->u.literal; *p != '\0'; p++) {
				if (*p == '\"') {
					fputs("\"\"", stdout);
				} else {
					putc(*p, stdout);
				}
			}
			putc('\"', stdout);
		}
		break;

	case TYPE_GROUP:
		output_alt(term->u.group);
		break;
	}

	printf("%s", e);
}

static void
output_alt(struct ast_alt *alt)
{
	struct ast_term *term;

	for (term = alt->terms; term != NULL; term = term->next) {
		output_term(term);
	}
}

static void
output_production(struct ast_production *production)
{
	struct ast_alt *alt;

	alt = production->alts;
	printf("%s =", production->name);
	output_alt(alt);

	for (alt = alt->next; alt != NULL; alt = alt->next) {
		printf("\n\t|");
		output_alt(alt);
	}

	printf(" .\n\n");
}

void
wsn_output(struct ast_production *grammar)
{
	struct ast_production *p;

	for (p = grammar; p != NULL; p = p->next) {
		output_production(p);
	}
}

