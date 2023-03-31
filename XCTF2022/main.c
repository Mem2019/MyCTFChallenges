#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <sys/resource.h>

typedef enum _ExpType
{
	kSymbol, kLambda, kCall
}ExpType;

typedef unsigned char Symbol;
typedef struct _Exp Exp;
typedef struct _Closure Closure;
typedef struct _Env Env;
typedef struct _LazyArg LazyArg;

// Expression represents AST of lambda calculus
struct _Exp
{
	ExpType type;
	union
	{
		Symbol symbol;
		struct
		{
			Symbol arg;
			const Exp* body;
		}lambda;
		struct
		{
			const Exp* rator;
			const Exp* rand;
		}call;
	}u;
};

struct _Closure
{
	const Exp* body;
	Symbol x;
	Env* env;
	size_t refcount;
};

struct _LazyArg
{
	// If `evaluated` is true, use `result`, otherwise use `lazy`
	bool evaluated;
	union
	{
		struct
		{
			const Exp* expr;
			Env* env;
		}lazy;
		Closure* result;
	}u;
};

struct _Env
{
	Symbol symbol;
	LazyArg* value;
	Env* next;
	size_t refcount;
};

/*
Rule of reference counter:
1. When pointer is copied to either local variable or heap field, counter increments.
2. When domain local variable finishes, counter decrements.
3. When heap object is released, counters of all fields decrement.
4. When local variable is returned, we don't decrement counter,
	but caller must receive the return value without increment,
	then it is regarded as a normal local variable with one reference count.
5. When pointer is passed as argument, it does not increment,
	because argument does not have any ownership.
*/

void delClosure(Closure* clos);
void delEnv(Env* env);

#define REF_COUNT_OPS(Type) \
Type* incRef##Type(Type* o) \
{ \
	if (o == NULL) return NULL; \
	++o->refcount; return o; \
} \
void decRef##Type(Type* o) \
{ \
	if (o == NULL) return; \
	if (--o->refcount == 0) \
		del##Type(o); \
}

REF_COUNT_OPS(Env)
REF_COUNT_OPS(Closure)

void delClosure(Closure* clos)
{
	// when body is NULL, it is our hand crafted closure, so don't free
	if (clos->body == NULL)
		return;
	decRefEnv(clos->env);
	free(clos);
}

void delEnv(Env* env)
{
	decRefEnv(env->next);
	if (env->value->evaluated)
	{
		decRefClosure(env->value->u.result);
	}
	else
	{
		decRefEnv(env->value->u.lazy.env);
	}
	free(env->value);
	free(env);
}

Env* pushEnv(Env* next, Symbol symbol, Env* env, const Exp* expr)
{
	Env* ret = malloc(sizeof(Env));
	ret->symbol = symbol;
	ret->value = malloc(sizeof(LazyArg));
	ret->value->evaluated = false;
	ret->value->u.lazy.env = incRefEnv(env);
	ret->value->u.lazy.expr = expr;
	ret->next = incRefEnv(next);
	ret->refcount = 1; // reference of `ret`
	return ret;
}

Env* pushClosureEnv(Env* next, Symbol symbol, Closure* result)
{
	Env* ret = malloc(sizeof(Env));
	ret->symbol = symbol;
	ret->value = malloc(sizeof(LazyArg));
	ret->value->evaluated = true;
	ret->value->u.result = incRefClosure(result);
	ret->next = incRefEnv(next);
	ret->refcount = 1;
	return ret;
}

Closure* valOf(const Exp* exp, Env* env)
{
	Closure* ret = NULL;
	switch (exp->type)
	{
		case kSymbol:
		{
			Symbol s = exp->u.symbol;
			// Iterate without ownership, so we don't increment counter
			for (Env* iter = env; iter != NULL; iter = iter->next)
			{
				if (iter->symbol == s)
				{
					LazyArg* v = iter->value;
					if (v->evaluated)
					{
						ret = incRefClosure(v->u.result);
						break;
					}
					else
					{
						ret = valOf(v->u.lazy.expr, v->u.lazy.env);
						decRefEnv(v->u.lazy.env);
						v->evaluated = true;
						v->u.result = incRefClosure(ret);
						break;
					}
				}
			}
			if (ret == NULL)
				abort();
		}
		break;
		case kLambda:
		{
			ret = malloc(sizeof(Closure));
			ret->body = exp->u.lambda.body;
			ret->x = exp->u.lambda.arg;
			ret->env = incRefEnv(env);
			ret->refcount = 1;
		}
		break;
		case kCall:
		{
			Closure* func = valOf(exp->u.call.rator, env);
			Env* newEnv = pushEnv(func->env, func->x, env, exp->u.call.rand);
			ret = valOf(func->body, newEnv);
			decRefEnv(newEnv);
			decRefClosure(func);
		}
		break;
		default:
		abort();
	}
	return ret;
}

Closure* val(const Exp* exp)
{
	return valOf(exp, NULL);
}

bool churchBool(Closure* clos)
{
	Closure tmp[2] = {0};
	Env* env = pushClosureEnv(clos->env, clos->x, tmp + 1);
	Closure* clos2 = valOf(clos->body, env);
	Env* env2 = pushClosureEnv(clos2->env, clos2->x, tmp);
	Closure* ret = valOf(clos2->body, env2);
	decRefClosure(clos);
	decRefClosure(clos2);
	decRefEnv(env);
	decRefEnv(env2);
	assert(ret == tmp || ret == tmp + 1);
	return ret == tmp + 1;
}

void wrong()
{
	printf("Wrong flag!\n");
	exit(-1);
}

#include "expr.h"

#define LC_STACK_SIZE (1024 * 1024 * 1024)
void setStack()
{
	struct rlimit rl;
	int ret = getrlimit(RLIMIT_STACK, &rl);
	if (ret != 0)
	{
		fprintf(stderr, "getrlimit failed\n");
		exit(-1);
	}
	// printf("rl.rlim_cur == %lu\n", rl.rlim_cur);
	if (rl.rlim_cur >= LC_STACK_SIZE)
		return;
	rl.rlim_cur = LC_STACK_SIZE;
	ret = setrlimit(RLIMIT_STACK, &rl);
	if (ret != 0)
	{
		fprintf(stderr, "setrlimit failed\n");
		exit(-1);
	}
}

Exp* encode(char c)
{
	if (c < 0)
		return NULL;
	size_t len = c + 2 + 2;
	Exp* ret = malloc(sizeof(Exp) * len);
	ret[0].type = kLambda; 
	ret[0].u.lambda.arg = 0; 
	ret[0].u.lambda.body = ret + len - 1;
	ret[1].type = kSymbol; ret[1].u.symbol = 0; // f
	ret[2].type = kSymbol; ret[2].u.symbol = 1; // x
	Exp* last = ret + 2;
	Exp* f = ret + 1;
	for (size_t i = 0; i < c; ++i)
	{
		ret[3 + i].type = kCall;
		ret[3 + i].u.call.rator = f;
		ret[3 + i].u.call.rand = last;
		last = ret + 3 + i;		
	}
	ret[len-1].type = kLambda; 
	ret[len-1].u.lambda.arg = 1; 
	ret[len-1].u.lambda.body = last;
	return ret;
}

int main(int argc, char const *argv[])
{
	if (argc != 2)
	{
		fprintf(stderr, "Usage: ./lambda \"XCTF{flag}\"\n");
		return -1;
	}
	setStack();
	if (strlen(argv[1]) != 33)
		wrong();
	for (size_t i = 0; i < 11; ++i)
	{
		printf("You got %lu/11 part of flag correct\n", i);
		Exp* seg[3];
		for (size_t j = 0; j < 3; ++j)
			seg[j] = encode(argv[1][i*3+j]);
		inputs[i][0]->u.call.rand = seg[0];
		inputs[i][1]->u.call.rand = seg[1];
		inputs[i][2]->u.call.rand = seg[2];
		if (!churchBool(val(chall[i])))
			wrong();
		for (size_t j = 0; j < 3; ++j)
			free(seg[j]);
	}
	puts("Congratulation! Submit the argument as the flag!");
	return 0;
}

// gcc -no-pie -fno-stack-protector -O0 -Wall -g main.c -o lambda