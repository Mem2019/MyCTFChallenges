#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>

typedef enum _ExpType
{
	kSymbol, kLambda, kCall
}ExpType;

typedef unsigned char Symbol;

typedef struct _Exp
{
	ExpType type;
	union
	{
		Symbol symbol;
		struct
		{
			Symbol arg;
			const struct _Exp* body;
		}lambda;
		struct
		{
			const struct _Exp* rator;
			const struct _Exp* rand;
		}call;
	}u;
}Exp;


typedef struct _Env Env;

typedef struct _Closure
{
	const Exp* body;
	Symbol x;
	const Env* env;
}Closure;

typedef struct _Env
{
	Symbol symbol;
	const Closure* value;
	const struct _Env* next;
}Env;

const Env* pushEnv(const struct _Env* next, Symbol symbol, const Closure* value)
{
	Env* ret = malloc(sizeof(Env));
	ret->symbol = symbol;
	ret->value = value;
	ret->next = next;
	return ret;
}

const Closure* valOf(const Exp* exp, const Env* env)
{
	switch (exp->type)
	{
		case kSymbol:
		{
			Symbol s = exp->u.symbol;
			for (const Env* iter = env; iter != NULL; iter = iter->next)
			{
				if (iter->symbol == s)
					return iter->value;
			}
			abort();
		}
		case kLambda:
		{
			Closure* ret = malloc(sizeof(Closure));
			ret->body = exp->u.lambda.body;
			ret->x = exp->u.lambda.arg;
			ret->env = env;
			return ret;
		}
		case kCall:
		{
			const Closure* func = valOf(exp->u.call.rator, env);
			const Closure* arg = valOf(exp->u.call.rand, env);
			return valOf(func->body, pushEnv(func->env, func->x, arg));
		}
		default:
		abort();
	}
}

const Closure* val(const Exp* exp)
{
	return valOf(exp, NULL);
}

bool churchBool(const Closure* clos)
{
	Closure tmp[2] = {0};
	const Closure* clos2 = valOf(clos->body,
		pushEnv(clos->env, clos->x, tmp + 1));
	const Closure* ret = valOf(clos2->body,
		pushEnv(clos2->env, clos2->x, tmp));
	assert(ret == tmp || ret == tmp + 1);
	return ret == tmp + 1;
}
/*
Exp churchTrue[] =
	{{.type = kLambda, .u = {.lambda = {.arg = 1, .body = churchTrue + 1}}},
	{.type = kLambda, .u = {.lambda = {.arg = 0, .body = churchTrue + 2}}},
	{.type = kSymbol, .u = {.symbol = 1}}};
Exp churchFalse[] =
	{{.type = kLambda, .u = {.lambda = {.arg = 1, .body = churchFalse + 1}}},
	{.type = kLambda, .u = {.lambda = {.arg = 0, .body = churchFalse + 2}}},
	{.type = kSymbol, .u = {.symbol = 0}}};
Exp churchAnd[] =
	{{.type = kLambda, .u = {.lambda = {.arg = 1, .body = churchAnd + 1}}},
	{.type = kLambda, .u = {.lambda = {.arg = 0, .body = churchAnd + 5}}},
	{.type = kSymbol, .u = {.symbol = 1}},
	{.type = kSymbol, .u = {.symbol = 0}},
	{.type = kCall, .u = {.call = {.rator = churchAnd + 2, .rand = churchAnd + 3}}},
	{.type = kCall, .u = {.call = {.rator = churchAnd + 4, .rand = churchAnd + 2}}}};
Exp churchOr[] =
	{{.type = kLambda, .u = {.lambda = {.arg = 1, .body = churchOr + 1}}},
	{.type = kLambda, .u = {.lambda = {.arg = 0, .body = churchOr + 5}}},
	{.type = kSymbol, .u = {.symbol = 1}},
	{.type = kSymbol, .u = {.symbol = 0}},
	{.type = kCall, .u = {.call = {.rator = churchOr + 2, .rand = churchOr + 2}}},
	{.type = kCall, .u = {.call = {.rator = churchOr + 4, .rand = churchOr + 3}}}};
Exp test[] =
	{{.type = kCall, .u = {.call = {.rator = test + 1, .rand = churchTrue}}},
	{.type = kCall, .u = {.call = {.rator = churchOr, .rand = churchTrue}}}};
//*/

#include "expr.h"

void wrong()
{
	printf("Wrong flag!\n");
	exit(-1);
}

int main(int argc, char const *argv[])
{
	puts("Input your flag: ");
	char flag[0x100] = {0};
	fgets(flag, sizeof(flag), stdin);
	size_t len = strlen(flag);
	if (len != 0x31)
		wrong();
	flag[0x30] = 0;
	for (size_t i = 0; i < 0x30; ++i)
	{
		for (size_t j = 0; j < 8; ++j)
		{
			size_t idx = INPUT_START + (i * 8 + j) * 3;
			assert(expr[idx].type == kSymbol && expr[idx].u.symbol == 137);
			expr[idx].u.symbol = (flag[i] >> j) & 1;
		}
	}
	if (churchBool(val(expBegin)))
		puts("Correct flag!");
	else
		wrong();
	return 0;
}

