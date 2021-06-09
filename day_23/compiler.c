#if 0 // A poor man's build system & runner
set -e

dir=$(mktemp -d workspace.XXXXXX)  # temporary workspace
trap "rm -r $dir" EXIT             # clean up afterwards

cc "$0" -o $dir/compiler       # build the compiler
./$dir/compiler > $dir/prog.c  # compile the program to C, reads from stdin
cc $dir/prog.c -o $dir/prog    # compile the compiled program
./$dir/prog                    # run the binary

exit  # don't try running arbitrary C code in the shell
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HASH(a, b, c) (((c)<<16)+((b)<<8)+(a))

#define failf(fmt, ...) (fprintf(stderr, "FAIL: " fmt "\n", __VA_ARGS__), exit(1))
#define fail(msg) failf("%s", msg)
#define assert(cond, msg) (void)(cond ? 0 : (failf("ASSERT: %s", msg), 0))

void *xalloc(int n) {
	void *p = calloc(1, n);
	if (!p)
		fail("alloc");
	return p;
}

#define NEW(t, ...) memcpy(xalloc(sizeof(t)), &(t){__VA_ARGS__}, sizeof(t))

#define FOR_EACH(x) for (; x; x = x->next)

const char *fname;

enum OPS {
	JNZ = HASH('j','n','z'),
	SET = HASH('s','e','t'),
	MUL = HASH('m','u','l'),
	SUB = HASH('s','u','b'),
};

typedef struct Expr {
	enum {
		BIN,
		VAL,
		REG,
	} ty;
	union {
		int32_t val;
		char reg;
		struct {
			enum {
				O_SUB = SUB,
				O_MUL = MUL,
				O_ADD,
				O_NOT,
				O_MOD,
				O_EQ,
				O_NE,
				O_RANGE,
				O_ITER,
				O_CALL,
			} op;
			struct Expr *lhs;
			struct Expr *rhs;
			int32_t func;
		};
	};
} Expr;


Expr *expr_bin(int32_t op, Expr *lhs, Expr *rhs) {
	return NEW(Expr, .op = op, .ty = BIN, .lhs = lhs, .rhs = rhs);
}

Expr *expr_reg(char reg) {
	return NEW(Expr, .ty = REG, .reg = reg);
}

Expr *expr_val(int32_t val) {
	return NEW(Expr, .ty = VAL, .val = val);
}

typedef struct Ast {
	enum {
		PROG,
		CODE,
		LOOP,
		DO_WHILE,
		FOR,
		IF,
		RET,
		NOP,
		DECL,
		CONST,
		S_JNZ = JNZ,
		S_SET = SET,
	} ty;
	int32_t id;
	struct Ast *next;
	union {
		const char *code;
		struct {
			struct Ast *root;
			struct Ast *decls;
		};
		struct {
			char reg;
			struct Expr *rhs;
		};
		struct {
			struct Expr *cond;
			struct Ast *body;
		};
	};
} Ast;

Ast *stmt(int32_t id, char reg, int32_t op, Expr *rhs) {
	return NEW(Ast, .id = id, .reg = reg, .ty = op, .rhs = rhs);
}

Ast *jump(int32_t id, Expr *cond, char offset) {
	return NEW(Ast, .id = id, .rhs = cond, .ty = JNZ, .reg = offset);
}

#define indent printf("%*s", 4*ind, "")
#define gen(...) printf(__VA_ARGS__)
#define genln(...) (indent, gen(__VA_ARGS__), printf(";\n"))

void gen_expr(Expr *e);

#define BIN_OP(op,left,right) (gen_expr(left), gen(op), gen_expr(right))

void gen_bin_op(Expr *e) {
	assert(e->ty == BIN, "bin op required");
	switch (e->op) {
		case O_MUL:
			BIN_OP("*", e->lhs, e->rhs);
			break;
		case O_SUB:
			BIN_OP("-", e->lhs, e->rhs);
			break;
		case O_ADD:
			BIN_OP("+", e->lhs, e->rhs);
			break;
		case O_MOD:
			BIN_OP("%%", e->lhs, e->rhs);
			break;
		case O_NOT:
			gen("!(");
			gen_expr(e->rhs);
			gen(")");
			break;
		case O_EQ:
			BIN_OP(" == ", e->lhs, e->rhs);
			break;
		case O_NE:
			BIN_OP(" != ", e->lhs, e->rhs);
			break;
		case O_ITER:
			BIN_OP(" : ", e->lhs, e->rhs);
			break;
		case O_RANGE:
			BIN_OP("..", e->lhs, e->rhs);
			break;
		case O_CALL:
			gen("%s_%c(", fname, e->func);
			BIN_OP(", ", e->lhs, e->rhs);
			gen(")");
			break;
		default:
			printf("%d\n", e->op);
			assert(0, "not handled bin op");
			break;
	}
}
#undef BIN_OP

void gen_expr(Expr *e) {
	switch (e->ty) {
		case VAL:
			gen("%d", e->val);
			break;
		case REG:
			gen("%s_%c", fname, e->reg);
			break;
		case BIN:
			gen_bin_op(e);
			break;
	}
}

void stmt_dump(Ast *t, int ind) {
	if (!t) {
		return;
	}
	switch (t->ty) {
		case S_SET:
			indent;
			gen("%s_%c = ", fname, t->reg);
			gen_expr(t->rhs);
			gen(";\n");
			break;
		case S_JNZ:
			indent;
			gen(":: jnz [");
			gen_expr(t->rhs);
			gen("] %d\n", t->reg);
			break;
		case RET:
			genln("return %s_R", fname);
			break;
		case CODE:
			gen("%s\n", t->code);
			break;
		case PROG:
			genln("// prog");
			break;
		case CONST:
			indent;
			gen("int const %s_%c = ", fname, t->reg);
			gen_expr(t->rhs);
			gen(";\n");
			break;
		case DECL:
			indent;
			gen("int %s_%c = ", fname, t->reg);
			gen_expr(t->rhs);
			gen(";\n");
			break;
		case DO_WHILE:
			genln("// do while");
			break;
		case LOOP:
			genln("// loop");
			break;
		case IF:
			genln("// if");
			break;
		case NOP:
			genln("// nop");
			break;
		case FOR:
			genln("// for");
			break;
	}
}

void ast_dump(Ast *t, int ind);

void block_dump(Ast *t, int ind) {
	gen("{\n");
	FOR_EACH(t) {
		ast_dump(t, ind+1);
	}
	indent;
	gen("}");
}

void ast_dump(Ast *t, int ind) {
	if (!t)
		return;
	switch (t->ty) {
		case PROG:
			for (Ast *p = t->decls; p; p = p->next) {
				ast_dump(p, 0);
			}
			gen("int %s() ", fname);
			block_dump(t->root, 0);
			gen("\n");
			break;
		case DO_WHILE:
			indent;
			gen("do ");
			block_dump(t->body, ind);
			gen(" while (");
			gen_expr(t->cond);
			gen(");\n");
			break;
		case IF:
			indent;
			gen("if (");
			gen_expr(t->cond);
			gen(") ");
			block_dump(t->body, ind);
			gen("\n");
			break;
		case LOOP:
			indent;
			gen("while (1) ");
			block_dump(t->body, ind);
			gen("\n");
			break;
		case FOR:
			indent;
			gen("for (int ");
			gen_expr(t->cond->lhs);
			gen(" = ");
			gen_expr(t->cond->rhs->lhs);
			gen("; ");
			gen_expr(t->cond->lhs);
			gen(" < ");
			gen_expr(t->cond->rhs->rhs);
			gen("; ");
			gen_expr(t->cond->lhs);
			gen("++) ");
			block_dump(t->body, ind);
			gen("\n");
			break;
		default:
			stmt_dump(t, ind);
			break;
	}
}

Expr *scan_val() {
	int32_t n;
	if (scanf("%d", &n) == 1)
		return expr_val(n);
	if (scanf(" %c", (char*)&n) == 1)
		return expr_reg(n);
	return NULL;
}

Ast *parse_line(int32_t id) {
	int32_t op;
	Expr *left, *right;
	if (scanf(" %3s", (char*)&op) != 1 || !(left = scan_val()) || !(right = scan_val()))
		return 0;
	char *c = (char*)&op;
	op = HASH(c[0],c[1],c[2]);
	switch (op) {
		case JNZ:
			if (right->ty != VAL)
				fail("jump offset must be value");
			return jump(id, left, right->val);
		case SUB:
			if (left->ty != REG)
				fail("target must be register");
			if (right->ty == VAL && right->val < 0) {
				op = O_ADD;
				right->val *= -1;
			}
			right = expr_bin(op, expr_reg(left->reg), right);
			return stmt(id, left->reg, S_SET, right);
		case MUL:
			if (left->ty != REG)
				fail("target must be register");
			right = expr_bin(O_MUL, expr_reg(left->reg), right);
			return stmt(id, left->reg, S_SET, right);
		case SET:
			if (left->ty != REG)
				fail("target must be register");
			return stmt(id, left->reg, S_SET, right);
		default:
			failf("bad op '%s'", (char*)&op);
			break;
	}
	return NULL;
}

Ast *parse() {
	int32_t id = 0;
	Ast *root = parse_line(++id);
	Ast *prev = root;
	FOR_EACH(prev) {
		prev->next = parse_line(++id);
	}
	return NEW(Ast, .id = 0, .ty = PROG, .root = root);
}

int has_body(Ast *p) {
	switch (p->ty) {
		case IF:
		case FOR:
		case LOOP:
		case DO_WHILE:
			return 1;
		default:
			return 0;
	}
}

Ast *find_by_id(Ast *t, int32_t id) {
	FOR_EACH(t) {
		if (t->id == id) {
			return t;
		}
	}
	return NULL;
}

int set_prev_next(Ast *r, Ast *t, Ast *next) {
	FOR_EACH(r) {
		if (r->next == t) {
			r->next = next;
			return 1;
		}
		if (has_body(r)) {
			if (r->body == t) {
				r->body = next;
				return 1;
			}
			if (set_prev_next(r->body, t, next)) {
				return 1;
			}
		}
	}
	return 0;
}

void *jmp_to_if(Ast *t) {
	assert(t, "null jmp to if");
	Ast old = *t;
	Ast *common = old.next, *prev = &old;
	while (--old.reg) {
		if (!common->next) {
			t->ty = IF;
			if (old.rhs->ty == BIN && old.rhs->op == O_NOT) {
				t->cond = old.rhs->rhs;
			} else {
				t->cond = expr_bin(O_NOT, NULL, old.rhs);
			}
			t->body = NEW(Ast, .ty = RET);
			t->next = prev;
			return t->next;
		}
		if (common->ty == JNZ)
			return common->next;
		prev = common;
		common = common->next;
	}
	t->ty = IF;
	if (old.rhs->ty == BIN && old.rhs->op == O_NOT) {
		t->cond = old.rhs->rhs;
	} else {
		t->cond = expr_bin(O_NOT, NULL, old.rhs);
	}
	t->body = old.next;
	Ast *nop = NEW(Ast, .ty = NOP, .id = prev->id, .next = common);
	assert(nop->next, "nop must have next");
	t->next = nop;
	prev->next = NULL;
	return common->next;
}

void resolve_if(Ast *p) {
	while (p) {
		if (p->ty != JNZ || p->reg <= 0) {
			p = p->next;
			continue;
		}
		int neg_jmp = p->reg == 2
			&& p->next
			&& p->next->ty == JNZ
			&& p->next->reg > 0
			&& p->next->rhs->ty == VAL
			&& p->next->rhs->val;
		if (neg_jmp) {
			Ast *next = p->next;
			p->reg = next->reg;
			p->next = next->next;
			p->rhs = expr_bin(O_NOT, NULL, p->rhs);
			next->ty = NOP;
		} else {
			p = jmp_to_if(p);
		}
	}
}

void resolve_ret(Ast *p) {
	Ast *root = p;
	FOR_EACH(p) {
		if (p->ty == JNZ && !find_by_id(root, p->id + p->reg)) {
			Expr *c = p->rhs;
			p->ty = IF;
			p->cond = c;
			p->body = NEW(Ast, .ty = RET);
		}
	}
}

int is_value(Expr *e) {
	return e && e->ty == VAL;
}

Expr *expr_eval(Expr *e) {
	switch (e->ty) {
		case VAL:
		case REG:
			return e;
		case BIN:
			e->rhs = expr_eval(e->rhs);
			if (e->op == O_NOT) {
				if (is_value(e->rhs)) {
					e->rhs->val ^= 1;
					return e->rhs;
				}
			} else {
				e->lhs = expr_eval(e->lhs);
			}
			if (!is_value(e->lhs) || !is_value(e->rhs)) {
				return e;
			}
			switch (e->op) {
				case O_EQ:
					e->lhs->val = e->lhs->val == e->rhs->val;
					return e->lhs;
				case O_NE:
					e->lhs->val = e->lhs->val != e->rhs->val;
					return e->lhs;
				case O_SUB:
					e->lhs->val -= e->rhs->val;
					return e->lhs;
				case O_ADD:
					e->lhs->val += e->rhs->val;
					return e->lhs;
				case O_MUL:
					e->lhs->val *= e->rhs->val;
					return e->lhs;
				case O_MOD:
					e->lhs->val %= e->rhs->val;
					return e->lhs;
				case O_ITER:
				case O_RANGE:
				case O_NOT:
				case O_CALL:
					break;
			}
	}
	return e;
}

Ast *jmp_to_loop(Ast *t, Ast *root) {
	assert(t, "null jmp to loop");
	Ast old = *t;
	Ast *common = find_by_id(root, t->id + t->reg - 1);
	if (!common) {
		return t;
	}
	t->ty = DO_WHILE;
	t->cond = old.rhs;
	Expr *e = expr_eval(t->cond);
	if (is_value(e)) {
		t->ty = e->val ? LOOP : NOP;
	}
	t->body = common->next;
	common->next = t;
	t->next = old.next;
	common = t->body;
	FOR_EACH(common) {
		if (common->next == t) {
			break;
		}
	}
	assert(common, "loop must have prev");
	common->next = NULL;
	return t;
}

void resolve_nop_ifs(Ast *p, Ast *root) {
	FOR_EACH(p) {
		if (has_body(p)) {
			if (p->body->ty == IF && !p->body->next) {
				Expr *e = expr_eval(p->body->cond);
				if (is_value(e)) {
					if (e->val) {
						p->body = p->body->body;
					} else {
						p->ty = NOP;
					}
				}
			}
			resolve_nop_ifs(p->body, root);
		}
		if (p->ty == IF) {
			Expr *e = expr_eval(p->cond);
			if (is_value(e)) {
				if (e->val) {
					set_prev_next(root, p, p->body);
					while (p->body->next) {
						p->body = p->body->next;
					}
					p->body->next = p->next;
				} else {
					p->ty = NOP;
				}
			}
		}
	}
}

void resolve_nops(Ast *p, Ast *root) {
	if (p->ty == PROG) {
		FOR_EACH(p->root) {
			if (p->root->ty != NOP) {
				break;
			}
		}
		p = p->root;
	}
	FOR_EACH(p) {
		if (has_body(p)) {
			Ast *t = p->body;
			FOR_EACH(t) {
				if (t->ty != NOP) {
					break;
				}
			}
			p->body = t;
			resolve_nops(p->body, root);
		}
		if (p->ty == NOP) {
			set_prev_next(root, p, p->next);
		}
	}
}


void resolve_loop(Ast *p) {
	Ast *root = p;
	FOR_EACH(p) {
		if (p->ty == JNZ && p->reg < 0) {
			p = jmp_to_loop(p, root);
		}
	}
}

void resolve_set_set(Ast *p) {
	Ast *next;
	Expr *e;
	char reg;
	FOR_EACH(p) switch (p->ty) {
		case IF:
		case FOR:
		case LOOP:
		case DO_WHILE:
			resolve_set_set(p->body);
			break;
		case S_SET:
			reg = p->reg;
			next = p->next;
			if (!next || next->ty != S_SET) {
				break;
			}
			e = expr_eval(next->rhs);
			if (e->ty == REG && e->reg == reg) {
				next->rhs = expr_eval(p->rhs);
			}
			break;
		default:
			break;
	}
}

void resolve_set(Ast *p) {
	Ast *next;
	Expr *e;
	char reg;
	FOR_EACH(p) top: switch (p->ty) {
		case IF:
		case FOR:
		case LOOP:
		case DO_WHILE:
			resolve_set(p->body);
			break;
		case S_SET:
			reg = p->reg;
			next = p->next;
			if (!next || next->ty != S_SET || next->reg != reg) {
				break;
			}
			e = p->next->rhs;
			if (e->ty == REG && e->reg == reg) {
				p->rhs = e;
				p->next = p->next->next;
				goto top;
			} else if (e->ty == BIN && e->lhs->ty == REG && e->lhs->reg == reg) {
				p->rhs = expr_bin(e->op, p->rhs, e->rhs);
				p->next = p->next->next;
				goto top;
			}
		default:
			break;
	}
}

void sub_cond_eq(Ast *p) {
	while (p->cond->ty == BIN && p->cond->op == O_SUB) {
		p->cond = expr_bin(O_NE, p->cond->lhs, p->cond->rhs);
	}
}

void sub_cond_ne(Ast *p) {
	while (p->cond->ty == BIN && p->cond->op == O_NOT) {
		Expr *e = p->cond->rhs;
		if (e->ty != BIN || e->op != O_SUB) {
			return;
		}
		p->cond = expr_bin(O_EQ, e->lhs, e->rhs);
	}
}

void resolve_sub_cond(Ast *p) {
	FOR_EACH(p) {
		if (has_body(p)) {
			resolve_sub_cond(p->body);
			sub_cond_eq(p);
			sub_cond_ne(p);
		}
	}
}

int expr_subst(Expr *e, int32_t reg, Expr *s) {
	if (!e || !s) {
		return 0;
	}
	int subs = 0;
	switch (e->ty) {
		case BIN:
			subs += expr_subst(e->lhs, reg, s);
			subs += expr_subst(e->rhs, reg, s);
			break;
		case VAL:
			break;
		case REG:
			if (e->reg == reg) {
				*e = *s;
				subs++;
			}
			break;
	}
	return subs;
}

Ast *find_prev(Ast *r, Ast *t) {
	FOR_EACH(r) {
		if (r->next == t) {
			return r;
		}
		if (has_body(r)) {
			Ast *x = find_prev(r->body, t);
			if (x) {
				return x;
			}
		}
	}
	return NULL;
}

void resolve_cond(Ast *p, Ast *root) {
	FOR_EACH(p) {
		if (has_body(p)) {
			resolve_cond(p->body, root);
		}
		if (p->ty == IF) {
			Ast *prev = find_prev(root, p);
			if (prev && prev->ty == S_SET) {
				if (expr_subst(p->cond, prev->reg, prev->rhs)) {
					prev->ty = NOP;
					set_prev_next(root, prev, p);
				}
			}
		} else if (p->ty == DO_WHILE) {
			Ast *last = p->body;
			while (last->next) {
				last = last->next;
			}
			if (last->ty == S_SET) {
				if (expr_subst(p->cond, last->reg, last->rhs)) {
					last->ty = NOP;
					set_prev_next(root, last, NULL);
				}
			}
		}
	}
}

int is_incr(Ast *s) {
	if (!s || s->ty != S_SET) {
		return 0;
	}
	Expr *e = s->rhs;
	if (e->ty != BIN || e->op != O_ADD) {
		return 0;
	}
	if (e->lhs->ty != REG || e->lhs->reg != s->reg) {
		return 0;
	}
	return e->rhs->ty == VAL && e->rhs->val == 1;
}

Expr *compares(Expr *e, char reg, int32_t op) {
	if (e->ty == BIN && (int)e->op == op) {
		Expr *l = e->lhs, *r = e->rhs;
		if (l->ty == REG && l->reg == reg) {
			return r;
		}
		if (r->ty == REG && r->reg == reg) {
			return l;
		}
	}
	return NULL;
}

int modifies(Ast *p, int32_t reg) {
	if (has_body(p)) {
		return modifies(p->body, reg);
	}
	switch (p->ty) {
		case S_SET:
		case DECL:
			return p->reg == reg;
		case CONST:
			assert(0, "what?");
		default:
			return 0;
	}
}

#if 0
for (i : 2..b) {
	if (x*i == b) {
		flag = val;
	}
}

// --- asserting x != b and x != 0

if (b % x == 0) {
	flag = val;
}
#endif

void resolve_mod(Ast *p) {
	int32_t i, b;
	Expr *e;
	FOR_EACH(p) {
		if (has_body(p)) {
			resolve_mod(p->body);
		}
		if (p->ty != FOR) {
			continue;
		}
		int should = p->body->ty == IF
			&& !p->body->next
			&& p->body->body->ty == S_SET
			&& !p->body->body->next
			&& is_value(p->body->body->rhs);
		if (!should) {
			continue;
		}
		e = p->cond->rhs;
		if (!is_value(e->lhs) || e->lhs->val != 2) {
			continue;
		}
		i = p->cond->lhs->reg;
		if (e->rhs->ty != REG) {
			continue;
		}
		b = e->rhs->reg;
		e = p->body->cond;
		e = compares(e, b, O_EQ);
		if (!e || e->ty != BIN || e->op != O_MUL) {
			continue;
		}
		if (e->lhs->ty == REG && e->lhs->reg == i) {
			e = expr_bin(O_MOD, expr_reg(b), e->rhs);
		} else if (e->rhs->ty == REG && e->rhs->reg == i) {
			e = expr_bin(O_MOD, expr_reg(b), e->lhs);
		}
		p->ty = IF;
		p->cond = expr_bin(O_NOT, NULL, e);
		p->body = p->body->body;
	}
}

void resolve_for(Ast *p, Ast *root) {
	Ast *prev = NULL;
	FOR_EACH(p) {
		if (has_body(p)) {
			resolve_for(p->body, root);
		}
		if (p->ty == DO_WHILE) {
			int should = prev
				&& prev->ty == S_SET
				&& is_value(prev->rhs);
			Expr *e;
			if (should) {
				e = compares(p->cond, prev->reg, O_NE);
				should = should && e && e->ty != BIN;
			}
			if (should) {
				int32_t r = prev->reg;
				Ast *last = p->body;
				while (last->next) {
					if (modifies(last, r) || (e->ty == REG && modifies(last, e->reg))) {
						last = NULL;
						break;
					}
					last = last->next;
				}
				if (is_incr(last)) {
					p->ty = FOR;
					e = expr_bin(O_RANGE, prev->rhs, e);
					p->cond = expr_bin(O_ITER, expr_reg(prev->reg), e);
					last->ty = NOP;
					prev->ty = NOP;
				}
			}
		}
		prev = p;
	}
}

typedef struct {
	struct _info {
		enum {
			V_SET   = 0x0001,
			V_RESET = 0x0010,
			V_USED  = 0x0100,
			V_CONST = 0x1000,
		} status;
		Ast *first_set;
		Ast *first_use;
	} i[256];
} VarInfo;

void expr_vars(Expr *e, Ast *from, VarInfo *i, int locals[256]) {
	int k;
	switch (e->ty) {
		case BIN:
			if (e->op != O_NOT) {
				expr_vars(e->lhs, from, i, locals);
			}
			expr_vars(e->rhs, from, i, locals);
			break;
		case REG:
			k = e->reg;
			if (!locals[k] && !(i->i[k].status & V_USED)) {
				i->i[k].status |= V_USED;
				i->i[k].first_use = from;
			}
			break;
		case VAL:
			break;
	}
}

void find_vars(Ast *p, VarInfo *i, int locals[256]) {
	int k;
	FOR_EACH(p) switch (p->ty) {
		case DECL:
		case CONST:
		case S_SET:
			k = p->reg;
			if (!(i->i[k].status & V_SET)) {
				i->i[k].status |= V_SET;
				i->i[k].first_set = p;
			} else {
				i->i[k].status |= V_RESET;
			}
			expr_vars(p->rhs, p, i, locals);
			break;
		case FOR:
			locals[(int)p->cond->lhs->reg] = 1;
			find_vars(p->body, i, locals);
			expr_vars(p->cond, p, i, locals);
			locals[(int)p->cond->lhs->reg] = 0;
			break;
		case IF:
		case DO_WHILE:
			expr_vars(p->cond, p, i, locals);
		case LOOP:
			find_vars(p->body, i, locals);
		default:
			break;
	}
}

void add_decl(Ast *p, Ast *d) {
	assert(p->ty == PROG, "must add decl on prog");
	if (!p->decls) {
		p->decls = d;
		return;
	}
	p = p->decls;
	while (p->next) {
		if (p->reg == d->reg) {
			return;
		}
		p = p->next;
	}
	if (p->reg != d->reg) {
		p->next = d;
	}
}

Ast *decl_expr(char reg, Expr *rhs, int is_const) {
	if (!rhs) {
		rhs = expr_val(0);
	}
	return NEW(Ast, .ty = is_const ? CONST : DECL, .reg = reg, .rhs = rhs);
}

void reset_vars(VarInfo *i) {
	for (int r = 0; r < 256; r++) {
		Ast *set = i->i[r].first_set;
		if (set) {
			set->ty = S_SET;
		}
		i->i[r] = (struct _info){0};
	}
}

void resolve_vars(Ast *p, VarInfo *i) {
	assert(p->ty == PROG, "must resolve vars on prog");

	reset_vars(i);
	find_vars(p->root, i, (int[256]){0});
	for (int r = 0; r < 256; r++) {
		if (i->i[r].status) {
			Ast *set = i->i[r].first_set;
			Ast *use = i->i[r].first_use;
			int c = !(i->i[r].status & V_RESET) && set != use;
			if (set && use && set->id < use->id) {
				Ast *t = i->i[r].first_set;
				t->ty = c ? CONST : DECL;
			} else if (set && !use) {
				set->ty = NOP;
				i->i[r] = (struct _info){0};
			} else {
				add_decl(p, decl_expr(r, NULL, c));
			}
			if (c) {
				i->i[r].status |= V_CONST;
			}
		}
	}
}

void expr_sub_reg(Expr *e, char reg, int32_t val) {
	if (!e) {
		return;
	}
	switch (e->ty) {
		case BIN:
			expr_sub_reg(e->lhs, reg, val);
			expr_sub_reg(e->rhs, reg, val);
			break;
		case REG:
			if (e->reg == reg) {
				e->ty = VAL;
				e->val = val;
			}
		case VAL:
			break;
	}
}

void sub_reg(Ast *p, char reg, int32_t val) {
	FOR_EACH(p) switch (p->ty) {
		case S_SET:
			expr_sub_reg(p->rhs, reg, val);
			break;
		case IF:
		case FOR:
		case DO_WHILE:
			expr_sub_reg(p->cond, reg, val);
		case LOOP:
			sub_reg(p->body, reg, val);
		default:
			break;
	}
}

void expr_sub_bin_op(Expr *e, int32_t op, int32_t func) {
	if (e->ty == BIN && e->op != O_NOT) {
		expr_sub_bin_op(e->lhs, op, func);
		expr_sub_bin_op(e->rhs, op, func);
		if ((int)e->op == op) {
			e->op = O_CALL;
			e->func = func;
		}
	}
}

void sub_bin_op(Ast *p, int32_t op, int32_t func) {
	FOR_EACH(p) switch (p->ty) {
		case S_SET:
			expr_sub_bin_op(p->rhs, op, func);
			break;
		case IF:
		case FOR:
		case DO_WHILE:
			expr_sub_bin_op(p->cond, op, func);
		case LOOP:
			sub_bin_op(p->body, op, func);
			break;
		default:
			break;
	}
}

Expr *expr_copy(Expr *p) {
	if (!p) {
		return NULL;
	}
	switch (p->ty) {
		case BIN:
			return expr_bin(p->op, expr_copy(p->lhs), expr_copy(p->rhs));
		case VAL:
			return expr_val(p->val);
		case REG:
			return expr_reg(p->reg);
	}
}

Ast *ast_copy(Ast *p) {
	if (!p) {
		return NULL;
	}
	Ast *s = NEW(Ast, .ty = p->ty, .id = p->id, .next = ast_copy(p->next));
	switch (p->ty) {
		case DECL:
		case CONST:
		case S_SET:
			s->reg = p->reg;
			s->rhs = expr_copy(p->rhs);
			break;
		case CODE:
			s->code = p->code;
			break;
		case RET:
			break;
		case PROG:
			s->root = ast_copy(p->root);
			s->decls = ast_copy(p->decls);
			break;
		case IF:
		case FOR:
		case LOOP:
		case DO_WHILE:
			s->cond = expr_copy(p->cond);
			s->body = ast_copy(p->body);
			break;
		case NOP:
		case S_JNZ:
			ast_dump(p, 0);
			assert(0, "bad ast node for copy");
			break;
	}
	return s;
}

void set_val(Expr *e, char r, int32_t v) {
	if (r) {
		expr_sub_reg(e, r, v);
		*e = *expr_eval(e);
	}
}

void propagate_val(Ast *p, char r, int32_t v) {
	assert(r, "null reg name");
	Ast *d = p;
	p = p->next;
	FOR_EACH(p) switch (p->ty) {
		case IF:
			set_val(p->cond, r, v);
		case FOR:
		case LOOP:
		case DO_WHILE:
			switch (p->body->ty) {
				case DECL:
				case CONST:
				case S_SET:
					if (is_value(p->body->rhs)) {
						propagate_val(p->body, p->body->reg, p->body->rhs->val);
					}
				default:
					break;
			}
			if (modifies(p, r)) {
				return;
			}
			break;
		case DECL:
		case CONST:
		case S_SET:
			set_val(p->rhs, r, v);
			if (p->reg == r) {
				if (!is_value(p->rhs)) {
					return;
				}
				p->ty = d->ty;
				v = p->rhs->val;
				d->ty = NOP;
				d = p;
			}
			break;
		default:
			return;
	}
}

void const_propagate(Ast *p, VarInfo *i) {
	resolve_vars(p, i);
	for (int r = 0; r < 256; r++) {
		Ast *a = i->i[r].first_set;
		if (a && is_value(a->rhs)) {
			propagate_val(a, a->reg, a->rhs->val);
		}
		if (i->i[r].status & V_CONST) {
			if (!a) {
				sub_reg(p->root, r, 0);
			} else {
				Expr *e = expr_eval(a->rhs);
				if (e->ty == VAL) {
					sub_reg(p->root, r, e->val);
				}
			}
		}
	}
}

void opt_run(Ast *ast) {
	resolve_ret(ast->root);
	resolve_if(ast->root);
	resolve_loop(ast->root);
	resolve_set(ast->root);
	resolve_set_set(ast->root);
	resolve_cond(ast->root, ast->root);
	resolve_sub_cond(ast->root);
	resolve_for(ast->root, ast->root);
	resolve_nop_ifs(ast->root, ast->root);
	resolve_nops(ast, ast->root);
}

int main() {
	fname = "main";
	Ast *ast = parse();
	opt_run(ast);
	Ast *copy = ast_copy(ast);

	VarInfo i = {{{0}}};
	for (int x = 0; x < 4; x++) {
		const_propagate(ast, &i);
		opt_run(ast);
	}

	add_decl(ast, decl_expr('R', NULL, 0));
	ast->decls = NEW(Ast, .ty = CODE, .code = "#define A_M(a,b) (A_R++,a*b)", .next = ast->decls);
	sub_bin_op(ast->root, O_MUL, 'M');

	gen("#include <stdio.h>\n");

	fname = "A";
	ast_dump(ast, 0);

	copy->decls = NEW(Ast, .ty = CODE, .code = "#define B_R B_h", .next = copy->decls);
	Ast *a = stmt(0, 'a', S_SET, expr_val(1));
	a->next = copy->root;
	copy->root = a;
	resolve_mod(copy->root);
	for (int x = 0; x < 4; x++) {
		const_propagate(copy, &i);
		opt_run(copy);
	}
	resolve_vars(copy, &i);

	fname = "B";
	ast_dump(copy, 0);

	gen("int main() {\n");
	int ind = 1;
	genln("printf(\"%%d\\n\", A())");
	genln("printf(\"%%d\\n\", B())");
	gen("}\n");
}
