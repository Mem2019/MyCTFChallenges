import random as rd
import numpy as np
import sys
import string

def genFlag(rand=False):
	if rand:
		i = ['l', 'I', 'i', '1', '|']
		flag = "XCTF{M4tR"
		for x in range(0,13):
			flag += i[rd.randint(0, 4)]
		flag += "X_A5_YC0mb}"
	else:
		flag = "XCTF{M4tRI1|i||l|Il|I1X_A5_YC0mb}"
	print(flag, file=sys.stderr)
	flag = list(map(ord, flag))
	return flag

# compile the lambda expression into C representation
# append result to dst, return index to expression
def compileLambda(expr, dst, loc, magic=0xDEFC0719600):
	if type(expr) == int:
		assert expr < 3 # 0 1 2 correspond to 3 inputs
		return magic + expr
		# encoding that represent input

	if expr[0] == "num":
		# For number, we want to prevent reaching recursion limit
		assert type(expr[2]) == int
		assert len(expr[1]) == 2
		f = len(dst)
		last = f + 1
		dst.append(("symbol", expr[1][0])) # f
		dst.append(("symbol", expr[1][1])) # x
		for i in range(0, expr[2]):
			dst.append(("call", f, last))
			last += 1
		dst.append(("lambda", expr[1][1], last))
		last += 1
		dst.append(("lambda", expr[1][0], last))
		ret = last + 1

	elif expr[0] == "lambda":
		body = compileLambda(expr[2], dst, loc, magic)
		if isinstance(expr[1], int):
			ret = len(dst)
			dst.append(("lambda", expr[1], body))
		else: # Support multiple-parameter lambda,
			# e.g. lambda (x,y) body == lambda (x) (lambda (y) body)
			last = body
			for a in reversed(expr[1]):
				dst.append(("lambda", a, last))
				last = len(dst) - 1
			ret = last

	elif expr[0] == "call": # Support multiple-argument lambda
		# e.g. (rator rand0 rand1) == ((rator rand0) rand1)
		assert len(expr) >= 3 # at least one argument
		arr = []
		for i in range(1, len(expr)):
			arr.append(compileLambda(expr[i], dst, loc, magic))
		last = arr[0]
		for i in range(1, len(arr)):
			dst.append(("call", last, arr[i]))
			last = len(dst) - 1
			if arr[i] >= magic:
				loc[arr[i] - magic] = last
		ret = last

	else:
		assert expr[0] == "symbol"
		ret = len(dst)
		dst.append(("symbol", expr[1]))

	return ret

def compileToC(macro, expr, magic=0xDEFC0719600):
	arr = []
	loc = dict()
	idx = compileLambda(expr, arr, loc, magic)
	ret = []
	name = "".join(rd.choices(string.ascii_lowercase, k=16))
	for e in arr:
		if e[0] == "lambda":
			assert e[2] < magic
			ret.append("{.type = kLambda, .u = {.lambda = {.arg = %u, .body = %s + %u}}}" % (e[1], name, e[2]))
		elif e[0] == "call":
			assert e[1] < magic
			if e[2] < magic:
				ret.append("{.type = kCall, .u = {.call = {.rator = %s + %u, .rand = %s + %u}}}" % (name, e[1], name, e[2]))
			else:
				ret.append("{.type = kCall, .u = {.call = {.rator = %s + %u, .rand = (Exp*)%u}}}" % (name, e[1], e[2]))
		else:
			ret.append("{.type = kSymbol, .u = {.symbol = %u}}" % e[1])
	ret = "Exp %s[] = {\n" % name + ",\n".join(ret) + "};\n"
	for k in loc:
		ret += "#define %s_LOC_INPUT%u (%s + %u)\n" % (macro, k, name, loc[k])
	ret += "#define %s (%s + %u)\n" % (macro, name, idx)
	return ret

def sym(s):
	return ["symbol", s]
def churchTrue():
	s = rd.sample(range(0, 0x100), 2)
	return ["lambda", s[1], ["lambda", s[0], sym(s[1])]]
def churchFalse():
	s = rd.sample(range(0, 0x100), 2)
	return ["lambda", s[1], ["lambda", s[0], sym(s[0])]]
def churchAnd():
	s = rd.sample(range(0, 0x100), 2)
	return ["lambda", s[1], ["lambda", s[0],
		["call", ["call", sym(s[1]), sym(s[0])], sym(s[1])]]]
def churchYComb():
	s = rd.sample(range(0, 0x100), 3)
	return ["lambda", s[0],
		["call",
			["lambda", s[1],
				["call", sym(s[0]), ["call", sym(s[1]), sym(s[1])]]],
			["lambda", s[2],
				["call", sym(s[0]), ["call", sym(s[2]), sym(s[2])]]]]]
def churchIf():
	s = rd.sample(range(0, 0x100), 3)
	return ["lambda", s, ["call", sym(s[0]), sym(s[1]), sym(s[2])]]
def churchPair():
	s = rd.sample(range(0, 0x100), 3)
	return ["lambda", s, ["call", sym(s[2]), sym(s[0]), sym(s[1])]]
def churchFirst():
	s = rd.sample(range(0, 0x100), 1)[0]
	return ["lambda", s, ["call", sym(s), churchTrue()]]
def churchSecond():
	s = rd.sample(range(0, 0x100), 1)[0]
	return ["lambda", s, ["call", sym(s), churchFalse()]]
def churchNil():
	s = rd.sample(range(0, 0x100), 1)[0]
	return ["lambda", s, churchTrue()]
def churchIsEmpty():
	s = rd.sample(range(0, 0x100), 3)
	return ["lambda", s[0],
		["call", sym(s[0]), ["lambda", [s[1], s[2]], churchFalse()]]]
def churchNum(val, opt=True):
	val = int(val)
	s = rd.sample(range(0, 0x100), 2)
	if opt:
		return ["num", s, val]
	else:
		body = sym(s[1])
		for i in range(0, val):
			body = ["call", sym(s[0]), body]
		return ["lambda", s, body]
def churchAdd():
	s = rd.sample(range(0, 0x100), 4)
	return ["lambda", s, ["call", sym(s[0]), sym(s[2]),
		["call", sym(s[1]), sym(s[2]), sym(s[3])]]]
def churchMul():
	s = rd.sample(range(0, 0x100), 4)
	return ["lambda", s, ["call", sym(s[0]),
		["call", sym(s[1]), sym(s[2])], sym(s[3])]]
def churchSucc():
	s = rd.sample(range(0, 0x100), 3)
	return ["lambda", s, ["call", sym(s[1]),
		["call", sym(s[0]), sym(s[1]), sym(s[2])]]]
def churchSub():
	p = rd.sample(range(0, 0x100), 1)[0]
	next_ = ["lambda", p, ["call",
		churchPair(),
		["call", churchSecond(), sym(p)],
		["call", churchSucc(), ["call", churchSecond(), sym(p)]]]]
	n = rd.sample(range(0, 0x100), 1)[0]
	pred_ = ["lambda", n,
		["call", churchFirst(),
			["call", sym(n), next_,
				["call", churchPair(), churchNum(0), churchNum(0)]]]]
	s = rd.sample(range(0, 0x100), 2)
	return ["lambda", s, ["call", sym(s[1]), pred_, sym(s[0])]]
def churchIsZero():
	s = rd.sample(range(0, 0x100), 2)
	return ["lambda", s[0], ["call", sym(s[0]),
		["lambda", s[1], churchFalse()], churchTrue()]]
def churchEq():
	s = rd.sample(range(0, 0x100), 2)
	return ["lambda", s,
		["call", churchAnd(),
			["call", churchIsZero(),
				["call", churchSub(), sym(s[0]), sym(s[1])]],
			["call", churchIsZero(),
				["call", churchSub(), sym(s[1]), sym(s[0])]]]]

churches = {
"True": churchTrue,
"And": churchAnd,
"YComb": churchYComb,
"If": churchIf,
"Pair": churchPair,
"First": churchFirst,
"Second": churchSecond,
"Nil": churchNil,
"IsEmpty": churchIsEmpty,
"Add": churchAdd,
"Mul": churchMul,
"Succ": churchSucc,
"Sub": churchSub,
"IsZero": churchIsZero,
"Eq": churchEq}


def buildList(arr):
	# arr should be array of lambda calculus expression
	ret = churchNil()
	for e in reversed(arr):
		ret = ["call", churchPair(), e, ret]
	return ret

def buildVec(vec):
	return buildList(list(map(churchNum, vec)))
def buildMat(mat):
	return buildList(list(map(buildVec, mat)))

def challCmp():
	s = rd.sample(range(0, 0x100), 3)
	rec = ["call", churchAdd(),
			["call", churchIf(),
				["call", churchEq(),
					["call", churchFirst(), sym(s[1])],
					["call", churchFirst(), sym(s[2])]],
				churchNum(1), churchNum(0)],
			["call", sym(s[0]),
				["call", churchSecond(), sym(s[1])],
				["call", churchSecond(), sym(s[2])]]]
	return ["call", churchYComb(),
		["lambda", s,
			["call", churchIf(),
				["call", churchIsEmpty(), sym(s[1])], churchNum(0), rec]]]

def challDot():
	s = rd.sample(range(0, 0x100), 4)
	mul = ["call", churchMul(),
			["call", churchAdd(),
				["call", churchFirst(), sym(s[1])],
				["call", churchSub(), 
					["call", churchMul(), churchNum(2), sym(s[3])],
					churchNum(5)]],
			["call", churchSub(),
				["call", churchFirst(), sym(s[2])],
				["call", churchAdd(), 
					churchNum(13),
					["call", churchSub(),
						churchNum(11),
						["call", churchMul(), churchNum(3), sym(s[3])]]]]]
	rec = ["call", sym(s[0]),
			["call", churchSecond(), sym(s[1])],
			["call", churchSecond(), sym(s[2])],
			["call", churchSucc(), sym(s[3])]]
	return ["call", churchYComb(),
		["lambda", s,
			["call", churchIf(),
				["call", churchIsEmpty(), sym(s[1])],
				churchNum(7),
				["call", churchAdd(), mul, rec]]]]

def challMat():
	s = rd.sample(range(0, 0x100), 3)
	pair = ["call", churchPair(),
			["call", challDot(),
				["call", churchFirst(), sym(s[1])],
				sym(s[2]), churchNum(2)],
			["call", sym(s[0]),
				["call", churchSecond(), sym(s[1])],
				sym(s[2])]]
	return ["call", churchYComb(),
		["lambda", s,
			["call", churchIf(),
				["call", churchIsEmpty(), sym(s[1])],
				churchNil(), pair]]]

def solve(mat, res):
	for i in range(0, 3):
		mat[i] += np.array([0, 1, 3], dtype='int')
	res -= 7
	return np.around(np.linalg.solve(mat, res) + \
		13 + np.array([5, 2, 0], dtype='int'))

def genMatRes(flag):
	size = len(flag)
	assert size == 3
	flagArr = np.array(flag, dtype='int') - 13 - np.array([5, 2, 0], dtype='int')
	assert (flagArr > 0).all()
	mat = np.random.randint(2, 8, size=(3, 3))
	matLambda = buildMat(mat)
	matBak = np.array(mat);
	for i in range(0, 3):
		mat[i] += np.array([0, 1, 3], dtype='int')
	res = np.matmul(mat, flagArr) + 7
	resLambda = buildVec(res)
	assert (solve(matBak, res) == flag).all()
	return matLambda, resLambda

def chall(m, r):
	return ["call", churchEq(), churchNum(3),
	["call", challCmp(),
		["call", challMat(), m, buildList(list(range(0, 3)))], r]]

if __name__ == '__main__':
	flag = genFlag()
	for i in range(0, 11):
		m, r = genMatRes(flag[i*3:i*3+3])
		print(compileToC("CHALL%u" % i, chall(m, r)))

	print("const Exp* chall[11] = {" + ','.join(["CHALL%u" % i for i in range(0, 11)]) + "};\n")
	print("Exp* inputs[11][3] = {" + ','.join( \
		["{CHALL%u_LOC_INPUT0, CHALL%u_LOC_INPUT1, CHALL%u_LOC_INPUT2}" % (i, i, i) \
			for i in range(0, 11)]) + "};\n")