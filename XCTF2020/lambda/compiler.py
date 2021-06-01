import random as rd
import sys

o = ['o', '0', 'O']
flag = "XCTF{14mBdA_C4LcU1u5_1MpL33mEn7_X"
for x in range(0,13):
	flag += o[rd.randint(0, 2)]
flag += "R}"
print(flag, file=sys.stderr)


invSeq = list(map(lambda _ : rd.randint(0, 1), range(0, len(flag) * 8)))
flag = list(map(ord, flag))

# compile the lambda expression into C representation
# append result to dst, return index to expression
def compileLambda(expr, dst, off):
	if type(expr) == int:
		return expr * 3 + off + 2
		# encoding that represent input
	if expr[0] == "lambda":
		body = compileLambda(expr[2], dst, off)
		ret = len(dst)
		dst.append(("lambda", expr[1], body))
	elif expr[0] == "call":
		rator = compileLambda(expr[1], dst, off)
		rand = compileLambda(expr[2], dst, off)
		ret = len(dst)
		dst.append(("call", rator, rand))
	else:
		assert expr[0] == "symbol"
		ret = len(dst)
		dst.append(("symbol", expr[1]))
	return ret

def compile(expr, numInputs, off=None):
	if off is None:
		arr = []
		idx = compileLambda(expr, arr, 0)
		off = len(arr)
	arr = []
	idx = compileLambda(expr, arr, off)
	arr += churchInputs(numInputs, off)
	ret = []
	for e in arr:
		if e[0] == "lambda":
			ret.append("{.type = kLambda, .u = {.lambda = {.arg = %u, .body = expr + %u}}}" % (e[1], e[2]))
		elif e[0] == "call":
			ret.append("{.type = kCall, .u = {.call = {.rator = expr + %u, .rand = expr + %u}}}" % (e[1], e[2]))
		else:
			ret.append("{.type = kSymbol, .u = {.symbol = %u}}" % e[1])
	return "Exp expr[] = {\n" + ",\n".join(ret) + ("};\nExp* expBegin = expr + %u;\n#define INPUT_START %u\n" % (idx, off))


def churchTrue():
	s = rd.sample(range(0, 0x100), 2)
	return ["lambda", s[1], ["lambda", s[0], ["symbol", s[1]]]]
def churchFalse():
	s = rd.sample(range(0, 0x100), 2)
	return ["lambda", s[1], ["lambda", s[0], ["symbol", s[0]]]]
def churchAnd():
	s = rd.sample(range(0, 0x100), 2)
	return ["lambda", s[1], ["lambda", s[0],
		["call", ["call", ["symbol", s[1]], ["symbol", s[0]]], ["symbol", s[1]]]]]
def churchOr():
	s = rd.sample(range(0, 0x100), 2)
	return ["lambda", s[1], ["lambda", s[0],
		["call", ["call", ["symbol", s[1]], ["symbol", s[1]]], ["symbol", s[0]]]]]
def churchNot():
	s = rd.sample(range(0, 0x100), 3)
	return ["lambda", s[2], ["lambda", s[1], ["lambda", s[0],
		["call", ["call", ["symbol", s[2]], ["symbol", s[0]]], ["symbol", s[1]]]]]]

# (or (and a (not b)) (and (not a) b))
def churchXor():
	s = rd.sample(range(0, 0x100), 2)
	# (and a (not b))
	and1 = ["call", ["call", churchAnd(), ["symbol", s[0]]], ["call", churchNot(), ["symbol", s[1]]]]
	# (and (not a) b)
	and2 = ["call", ["call", churchAnd(), ["call", churchNot(), ["symbol", s[0]]]], ["symbol", s[1]]]
	return ["lambda", s[1], ["lambda", s[0],
		["call", ["call", churchOr(), and1], and2]]]

def churchInputs(count, off=0):
	ret = []
	for i in range(0, count):
		if invSeq[i] == 1: # inverse
			ret += [('symbol', 137), ('lambda', 1, 0 + 3 * i + off), ('lambda', 0, 1 + 3 * i + off)]
		else: # not inverse
			ret += [('symbol', 137), ('lambda', 0, 0 + 3 * i + off), ('lambda', 1, 1 + 3 * i + off)]
	return ret

def challenge():
	andChain = churchTrue()
	for i in range(0, len(flag)):
		for j in range(0, 8):
			# if inverse:
			# 	not flag[i][j]
			# else:
			#	flag[i][j]
			b = (flag[i] >> j) & 1
			if invSeq[i * 8 + j]:
				b = not b
			xor = ["call", ["call", churchXor(), i * 8 + j],
				churchFalse() if b else churchTrue()]
			andChain = ["call", ["call", churchAnd(), andChain], xor]
	return compile(andChain, len(flag) * 8)



# print(compile(["call", ["call", churchOr(), 2], churchTrue()], 4))
# print(churchInputs(4))
print(challenge())

